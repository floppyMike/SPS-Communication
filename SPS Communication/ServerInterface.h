#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"
#include "VarSequences.h"

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
		if (const auto num = str_to_num<unsigned int>(r.get_var("data", "requesttimeout").GetString()); num.has_value())
			m_curr_timeout = std::chrono::seconds(num.value());
		else
			throw Logger("requesttimeout string unconvertable.");

		//Get authcode
		m_authcode = r.get_var("data", "authcode").GetString();
	}

	auto get_request()
	{
		//Validate and parse response
		basic_ResponseHandler<EDebugHandler, EDataHandler> r;
		r.go_through_content(this->_query<EGETBuilder>(this->host(), "/data.txt"));
			//{ q.host(this->host()).path("/interact.php").emplace_parameter("type", "raw").emplace_parameter("authcode", m_authcode); }));

		return this->_interpret_data(r.data());
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
				g_log.write("Failed getting authentication code. Error: " + std::string(e.what()) + '\n');
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
class EDataIntepreter
{
public:
	EDataIntepreter() = default;

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
				inpret.push_var(iter_var->name.GetString(), iter_var->value.GetString());

		return inpret.give();
	}

private:
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
