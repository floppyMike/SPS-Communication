#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"
#include "Connector.h"
#include "Interpeter.h"

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

class StockManager
{
public:
	StockManager()
		: m_stock(rj::Type::kObjectType)
	{
		m_stock.AddMember(
			rj::Value().SetString("settings", m_stock.GetAllocator()),
			rj::Value(rj::Type::kObjectType).AddMember(
				rj::Value().SetString("const", m_stock.GetAllocator()),
				1,
				m_stock.GetAllocator()
			)
			.AddMember(
				rj::Value().SetString("mutable", m_stock.GetAllocator()),
				2,
				m_stock.GetAllocator()
			),
			m_stock.GetAllocator()
		)
		.AddMember(
			rj::Value().SetString("data", m_stock.GetAllocator()),
			rj::Value(rj::Type::kObjectType),
			m_stock.GetAllocator()
		)
		.AddMember(
			rj::Value().SetString("usermodifiabledata", m_stock.GetAllocator()),
			rj::Value(rj::Type::kArrayType),
			m_stock.GetAllocator()
		)
		.AddMember(
			rj::Value().SetString("friendly", m_stock.GetAllocator()),
			rj::Value(rj::Type::kObjectType).AddMember(
				rj::Value().SetString("settingsvar", m_stock.GetAllocator()),
				rj::Value(rj::Type::kObjectType).AddMember(
					rj::Value().SetString("const", m_stock.GetAllocator()),
					rj::Value().SetString("Database number for the constant variables.", m_stock.GetAllocator()),
					m_stock.GetAllocator()
				)
				.AddMember(
					rj::Value().SetString("mutable", m_stock.GetAllocator()),
					rj::Value().SetString("Database number for the mutable variables.", m_stock.GetAllocator()),
					m_stock.GetAllocator()
				),
				m_stock.GetAllocator()
			)
			.AddMember(
				rj::Value().SetString("datavar", m_stock.GetAllocator()),
				rj::Value(rj::Type::kObjectType),
				m_stock.GetAllocator()
			)
			.AddMember(
				rj::Value().SetString("datavalue", m_stock.GetAllocator()),
				rj::Value(rj::Type::kObjectType),
				m_stock.GetAllocator()
			)
			.AddMember(
				rj::Value().SetString("type", m_stock.GetAllocator()),
				rj::Value().SetString("SPS", m_stock.GetAllocator()),
				m_stock.GetAllocator()
			),
			m_stock.GetAllocator()
		)
		.AddMember(
			rj::Value().SetString("preferredupdatetime", m_stock.GetAllocator()),
			10,
			m_stock.GetAllocator()
		);
	}

	JSONRoot generate_json_reply(const VarSequence& t) const
	{
		rj::Document doc;
		doc.CopyFrom(m_stock, doc.GetAllocator());

		// Add data vars
		auto& root_data = doc["data"];
		for (const auto& i : t)
			root_data.AddMember(
				rj::Value().SetString(i.name().data(), doc.GetAllocator()),
				rj::Value().SetString(i.val_str().data(), doc.GetAllocator()),
				doc.GetAllocator());

		// Add modifiables
		auto& root_modifiable = doc["usermodifiabledata"];
		for (const auto& i : t)
			root_modifiable.PushBack(
				rj::Value().SetString(i.name().data(), doc.GetAllocator()),
				doc.GetAllocator());

		// Add data variable discriptions
		auto& root_friendly_datavar = doc["friendly"]["datavar"];
		for (const auto& i : t)
			root_friendly_datavar.AddMember(
				rj::Value().SetString(i.name().data(), doc.GetAllocator()),
				rj::Value().SetString(std::string("Variable of type ").append(i.type_str()).data(), doc.GetAllocator()),
				doc.GetAllocator());

		// Add data value discriptions
		auto& root_friendly_datavalue = doc["friendly"]["datavalue"];
		for (const auto& i : t)
			root_friendly_datavalue.AddMember(
				rj::Value().SetString(i.name().data(), doc.GetAllocator()),
				rj::Value().SetString(std::string("Value of type ").append(i.type_str()).data(), doc.GetAllocator()),
				doc.GetAllocator());

		return JSONRoot(std::move(doc));
	}

	void update_stock(JSONValue root_device)
	{
		m_stock.AddMember(
			rj::Value().SetString("device", m_stock.GetAllocator()),
			rj::Value().CopyFrom(root_device.data(), m_stock.GetAllocator()),
			m_stock.GetAllocator());
	}

private:
	rj::Document m_stock;
};

class ServerInterface
	: Connector
	, StockManager
{
public:
	ServerInterface() = default;

	using Connector::io;
	using Connector::host;

	void pair_up()
	{
		//Validate and parse response
		auto json = ResponseHandler().parse_content(this->query<GETBuilder>("/pair.php",
			std::array{ Parameter{ "type", "raw" } }));

		m_curr_timeout = std::chrono::seconds(json.var("requesttimeout").num<unsigned int>());
		m_authcode = json.var("authcode").string();
	}

	auto get_request()
	{
		//Validate and parse response
		auto json = ResponseHandler().parse_content(this->query<GETBuilder>("/interact.php",
			std::array{ Parameter{ "type", "raw" }, Parameter{ "authcode", m_authcode }, Parameter{ "requesttype", "GET" } }));

		this->update_stock(json.var("data", "device"));

		m_curr_timeout = std::chrono::seconds(json.var("requesttimeout").num<unsigned int>());

		Interpreter inter;

		if (auto s = json.var("data").safe_var("settings"); s.has_value())
			inter.prepare_seqs(s.value());
		else
			inter.prepare_seqs();

		inter.prepare_vars("interpret");

		if (auto s = json.var("data").safe_var("data"); s.has_value())
			inter.fill_vars(s.value());

		return inter.give_seqs();
	}

	void post_request(const VarSequence& seq)
	{
		const auto str = this->generate_json_reply(seq).to_string();

		auto json = ResponseHandler().parse_content(this->query<POSTBuilder>("/interact.php", std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", "PUT" },
			Parameter{ "authcode", m_authcode }, Parameter{ "data", str } }));

		m_curr_timeout = std::chrono::seconds(json.var("requesttimeout").num<unsigned int>());
	}

	const auto& timeout() const noexcept { return m_curr_timeout; }

private:
	std::string m_authcode;
	std::chrono::seconds m_curr_timeout = 0s;

	JSONRoot m_stock;
};