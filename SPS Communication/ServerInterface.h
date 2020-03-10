#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"
#include "Sequencer.h"

template<template<typename> class... Ex>
class basic_ServerInterface : public Ex<basic_ServerInterface<Ex...>>...
{ //Requires: _query, host, _interpret_data
public:
	basic_ServerInterface() = default;

	void pair_up()
	{
		//Validate and parse response
		basic_ResponseHandler<EDebugHandler, EDataHandler> r;
		r.go_through_content(this->_query<EGETBuilder>(this->host(), "/pair.php", Parameter{ "type", "raw" }));

		//Convert time to seconds
		m_curr_timeout = std::chrono::seconds(guarded_get(str_to_num<unsigned int>(r.get_var("data", "requesttimeout").GetString()), "requesttimeout string unconvertable."));

		//Get authcode
		m_authcode = r.get_var("data", "authcode").GetString();
	}

	auto get_request()
	{
		//Validate and parse response
		basic_ResponseHandler<EDebugHandler, EDataHandler> r;
		r.go_through_content(this->_query<EGETBuilder>(this->host(), "/data.txt"));
			//{ q.host(this->host()).path("/interact.php").emplace_parameter("type", "raw").emplace_parameter("authcode", m_authcode); }));

		auto temp = this->_interpret_data(r.data());

		this->_store_prev(r.give_data());

		return temp;
	}

	template<typename VarSeq>
	void post_request(const VarSeq& seq)
	{
		const auto str = this->_to_json_str(seq);

		this->_query<EPOSTBuilder>(this->host(), "/interact.php", Parameter{ "type", "raw" }, Parameter{ "requesttype", "UPDATE" }, 
			Parameter{ "authcode", m_authcode }, Parameter{ "state", str });
	}

	const auto& timeout_dur() const noexcept { return m_curr_timeout; }

private:
	std::string m_authcode;
	std::chrono::seconds m_curr_timeout;
};

template<typename Impl>
class EConnector
{
	const Impl* underlying() const noexcept { return static_cast<Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	EConnector() = default;

	auto& io(asio::io_context& i) noexcept { m_io = &i; return *underlying(); }

	const auto& host() const noexcept { return m_host; }
	auto& host(std::string_view h) noexcept { m_host = h; return *underlying(); }

protected:
	template<template<typename> class Builder, typename... Args>
	std::string _query(Args&&... para)
	{
		//Construct query
		basic_Query<Builder, EParamBuilder> q;

		//Send query multiple times
		for (char test_case = 1; test_case <= 3; ++test_case)
		{
			try
			{
				return q.query<Session>(*m_io, std::forward<Args>(para)...).content;
			}
			catch (const std::exception & e)
			{
				g_log.write("Connection failed. Error: " + std::string(e.what()) + '\n');
				g_log.write(std::string("Trying again... ") + static_cast<char>(test_case + '0') + " of 3.\n");
			}
		}

		//Throw error at fail
		throw Logger("");
	}

private:
	asio::io_context* m_io;
	std::string_view m_host;		//Must be from main char**
};

template<typename Impl>
class EConnectorDEBUG
{
	const Impl* underlying() const noexcept { return static_cast<Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	EConnectorDEBUG() = default;

	auto& io(asio::io_context& i) noexcept { m_io = &i; return *underlying(); }

	const auto& host() const noexcept { return m_host; }
	auto& host(std::string_view h) noexcept { m_host = h; return *underlying(); }

protected:
	template<template<typename> class Builder, typename... Args>
	std::string _query(Args&&... para)
	{
		return _debug_filereader_("data.txt");
	}

private:
	asio::io_context* m_io;
	std::string_view m_host;		//Must be from main char**


	std::string _debug_filereader_(std::string_view name)
	{
		std::ifstream in(name.data(), std::ios::in | std::ios::binary);
		return (std::stringstream() << in.rdbuf()).str();
	}
};

template<typename Impl>
class EDataIntepreter
{
public:
	EDataIntepreter() = default;

	const auto& timeout() const noexcept { return m_timeout; }

protected:
	auto _interpret_data(const rapidjson::Document& dat)
		-> std::optional<VariableSequences<basic_VarSeq<Variable, EVarByteArray>>>
	{
		int var, perm;

		if (const auto loc = dat.FindMember("settings"); loc != dat.MemberEnd())
			if (const auto found = _db_exists_(loc->value); found.has_value())
				std::tie(var, perm) = _db_num_(found.value().first, found.value().second);
			else
				return std::nullopt;
		else
			return std::nullopt;

		Sequencer<basic_VarSeq<Variable, EVarByteArray>, EKeyedSorter> inpret(var, perm);
		auto& sec = dat["data"];

		//Insert to var sequence
		for (auto iter_var = sec.MemberBegin(), end = sec.MemberEnd(); iter_var != end; ++iter_var)
			if (!iter_var->name.IsString() && !iter_var->value.IsString())
				throw Logger("A data value isn't valid.");
			else
			{
				const auto name = iter_var->name.GetString();

				if (std::strcmp(name, "timeout") == 0)
					m_timeout = std::chrono::seconds(guarded_get(str_to_num<size_t>(iter_var->value.GetString()), "timeout value is unreadable."));

				else
					inpret.push_var(name, iter_var->value.GetString());
			}

		return inpret.give();
	}

private:
	std::chrono::seconds m_timeout;

	auto _db_exists_(const rapidjson::Value& sub)
		-> std::optional<std::pair<rapidjson::Value::ConstMemberIterator, rapidjson::Value::ConstMemberIterator>>
	{
		const auto var_loc = sub.FindMember("var"), perm_loc = sub.FindMember("perm");
		if (var_loc == sub.MemberEnd() || perm_loc == sub.MemberEnd())
			return std::nullopt;

		return std::pair(var_loc, perm_loc);
	}

	template<typename Iter>
	std::pair<int, int> _db_num_(Iter&& var_loc, Iter&& perm_loc)
	{
		if (var_loc->value.IsString() && perm_loc->value.IsString())
			if (const auto num_var = str_to_num<int>(var_loc->value.GetString()), num_perm = str_to_num<int>(perm_loc->value.GetString()); 
				num_var.has_value() && num_perm.has_value())
				return { num_var.value(), num_perm.value() };
			else
				throw Logger("Settings don't have numbers.");
		else
			throw Logger("Setting aren't holding numbers.");
	}

};

template<typename Impl>
class EJSONConverter
{
public:
	EJSONConverter() = default;

protected:
	void _store_prev(rapidjson::Document&& v)
	{
		m_prev_val = std::move(v);
	}

	template<typename VarSeq>
	std::string _to_json_str(const VarSeq& seq)
	{
		m_prev_val["data"].RemoveAllMembers();

		auto& data_sec = m_prev_val["data"];

		for (auto [iter_seq, num] = std::pair(seq.begin(), 0u); iter_seq != seq.end(); ++iter_seq, ++num)
		{
			const auto name = (std::to_string(seq.db()) + '_').append(iter_seq->type_str()) + '_' + std::to_string(num);
			const auto val = iter_seq->val_str();

			data_sec.AddMember(rapidjson::Value().SetString(name.c_str(), m_prev_val.GetAllocator()), 
				rapidjson::Value().SetString(val.c_str(), m_prev_val.GetAllocator()), m_prev_val.GetAllocator());
		}

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<decltype(buffer)> w(buffer);
		m_prev_val.Accept(w);

		return buffer.GetString();
	}

private:
	rapidjson::Document m_prev_val;
};
