#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"
#include "Sequencer.h"
#include "Connector.h"

class EDataIntepreter
{
public:
	EDataIntepreter() = default;

protected:
	auto _interpret_data(const rj::Document& dat)
		-> std::optional<VariableSequences<VarSequence>>
	{
		const auto& member_root = guarded_get_section(dat, "data");

		int var, perm;
		if (const auto result = _get_settings_(member_root); result.has_value())
			std::tie(var, perm) = result.value();
		else
			return std::nullopt;

		return _go_through_data_(var, perm, member_root);
	}

private:
	std::optional<std::pair<int, int>> _get_settings_(const rj::Value& root)
	{
		if (const auto& member_set = root.FindMember("settings"); member_set != root.MemberEnd())
		{
			const auto& member_var = guarded_get_section(member_set->value, "var"), 
				& member_perm = guarded_get_section(member_set->value, "perm");

			return std::pair{ guarded_get(str_to_num<int>(guarded_get_string(member_var)), "Value of \"var\" isn't a number."),
				guarded_get(str_to_num<int>(guarded_get_string(member_perm)), "Value of \"perm\" isn't a number.") };
		}
		else
			return std::nullopt;
	}

	VariableSequences<VarSequence>&& _go_through_data_(int var, int perm, const rj::Value& root)
	{
		Sequencer inpret(var, perm);
		const auto& member_data = guarded_get_section(root, "data");

		for (auto iter_var = member_data.MemberBegin(), end = member_data.MemberEnd(); iter_var != end; ++iter_var)
			inpret.push_var(guarded_get_string(iter_var->name), guarded_get_string(iter_var->value));

		return inpret.give();
	}

};

class EJSONConverter
{
public:
	EJSONConverter() = default;

	void replace_json_stock()
	{
		size_t flags = 0;
		constexpr size_t settings_f = 1;

		auto& data_root = guarded_get_section(m_prev_val, "data");

		for (auto iter = data_root.MemberBegin(); iter != data_root.MemberEnd(); ++iter)
			if (::strcmp(guarded_get_string(iter->name), "settings") == 0)
				flags |= settings_f;
			else if (::strcmp(guarded_get_string(iter->name), "friendly") == 0)
				data_root.RemoveMember(iter);

		if ((flags & settings_f) != settings_f)
			_add_stock_settings_();

		_add_stock_friend_();
	}

protected:
	void _store_prev(rj::Document&& v)
	{
		m_prev_val = std::move(v);
	}

	std::string _to_json_str(const VarSequence& seq)
	{
		m_prev_val["data"].RemoveAllMembers();

		auto& data_sec = m_prev_val["data"];

		for (auto [iter_seq, num] = std::pair(seq.begin(), 0u); iter_seq != seq.end(); ++iter_seq, ++num)
		{
			const auto name = (std::to_string(seq.db()) + '_').append(iter_seq->type_str()) + '_' + std::to_string(num);
			const auto val = iter_seq->val_str();

			data_sec.AddMember(rj::Value().SetString(name.c_str(), m_prev_val.GetAllocator()), 
				rj::Value().SetString(val.c_str(), m_prev_val.GetAllocator()), m_prev_val.GetAllocator());
		}

		return _to_json_str();
	}

	std::string _to_json_str()
	{
		rj::StringBuffer buffer;
		rj::Writer<decltype(buffer)> w(buffer);
		m_prev_val.Accept(w);

		return buffer.GetString();
	}

private:
	rj::Document m_prev_val;


	void _add_stock_settings_()
	{
		m_prev_val["data"].AddMember(
			rj::Value().SetString("settings", m_prev_val.GetAllocator()),
			rj::Value(rj::Type::kObjectType).AddMember(
				rj::Value().SetString("var", m_prev_val.GetAllocator()),
				1,
				m_prev_val.GetAllocator()
				)
			.AddMember(
				rj::Value().SetString("perm", m_prev_val.GetAllocator()),
				2,
				m_prev_val.GetAllocator()
				),
			m_prev_val.GetAllocator()
			);
	}

	void _add_stock_friend_()
	{
		m_prev_val["data"].AddMember(
			rj::Value().SetString("friendly", m_prev_val.GetAllocator()),
			rj::Value(rj::Type::kObjectType).AddMember(
				rj::Value().SetString("settingsvar", m_prev_val.GetAllocator()),
				rj::Value(rj::Type::kObjectType).AddMember(
					rj::Value().SetString("var", m_prev_val.GetAllocator()),
					rj::Value().SetString("Stores the database number of the modifiable variables.", m_prev_val.GetAllocator()),
					m_prev_val.GetAllocator()
					)
				.AddMember(
					rj::Value().SetString("perm", m_prev_val.GetAllocator()),
					rj::Value().SetString("Stores the database number of the constant variables.", m_prev_val.GetAllocator()),
					m_prev_val.GetAllocator()
					),
				m_prev_val.GetAllocator()
				)
			.AddMember(
				rj::Value().SetString("type", m_prev_val.GetAllocator()),
				rj::Value().SetString("SPS", m_prev_val.GetAllocator()),
				m_prev_val.GetAllocator()
				),
			m_prev_val.GetAllocator()
			);
	}
};

class ServerInterface
	: public EDataIntepreter
	, public EConnector<ServerInterface>
	, public EJSONConverter
{
public:
	ServerInterface() = default;

	void pair_up()
	{
		//Validate and parse response
		ResponseHandler r;
		r.go_through_content(this->template _query<GETBuilder>("/pair.php",
			std::array{ Parameter{ "type", "raw" } }));

		m_curr_timeout = std::chrono::seconds(guarded_get(str_to_num<unsigned int>(guarded_get_string(r.get_var("requesttimeout"))), "requesttimeout string unconvertable."));
		m_authcode = guarded_get_string(r.get_var("authcode"));
	}

	auto get_request()
	{
		//Validate and parse response
		ResponseHandler r;
		r.go_through_content(this->template _query<GETBuilder>("/interact.php",
			std::array{ Parameter{ "type", "raw" }, Parameter{ "authcode", m_authcode }, Parameter{ "requesttype", "GET" } }));

		m_curr_timeout = std::chrono::seconds(guarded_get(str_to_num<unsigned int>(guarded_get_string(r.get_var("requesttimeout"))), "requesttimeout string unconvertable."));

		auto temp = this->_interpret_data(r.data());
		this->_store_prev(r.give_data());

		return temp;
	}

	void post_request(const VarSequence& seq)
	{
		const auto str = this->_to_json_str(seq);

		this->_query<POSTBuilder>("/interact.php", std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", "UPDATE" },
			Parameter{ "authcode", m_authcode }, Parameter{ "data", str } });
	}
	void post_request()
	{
		const auto str = this->_to_json_str();

		this->_query<POSTBuilder>("/interact.php", std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", "UPDATE" },
			Parameter{ "authcode", m_authcode }, Parameter{ "data", str } });
	}

	const auto& timeout_dur() const noexcept { return m_curr_timeout; }

private:
	std::string m_authcode;
	std::chrono::seconds m_curr_timeout = 0s;
};