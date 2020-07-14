#pragma once

#include "Includes.h"
#include "JSON.h"
#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"

class StockManager
{
public:
	StockManager()
		: m_stock(rj::Type::kObjectType)
	{
		// Initialize the stock json data
		m_stock
			.AddMember(
				rj::Value().SetString("settings", m_stock.GetAllocator()),
				rj::Value(rj::Type::kObjectType)
					.AddMember(rj::Value().SetString("const", m_stock.GetAllocator()), 1, m_stock.GetAllocator())
					.AddMember(rj::Value().SetString("mutable", m_stock.GetAllocator()), 2, m_stock.GetAllocator()),
				m_stock.GetAllocator())
			.AddMember(rj::Value().SetString("data", m_stock.GetAllocator()), rj::Value(rj::Type::kObjectType),
					   m_stock.GetAllocator())
			.AddMember(rj::Value().SetString("usermodifiabledata", m_stock.GetAllocator()),
					   rj::Value(rj::Type::kArrayType), m_stock.GetAllocator())
			.AddMember(
				rj::Value().SetString("friendly", m_stock.GetAllocator()),
				rj::Value(rj::Type::kObjectType)
					.AddMember(rj::Value().SetString("settingsvar", m_stock.GetAllocator()),
							   rj::Value(rj::Type::kObjectType)
								   .AddMember(rj::Value().SetString("const", m_stock.GetAllocator()),
											  rj::Value().SetString("Database number for the constant variables.",
																	m_stock.GetAllocator()),
											  m_stock.GetAllocator())
								   .AddMember(rj::Value().SetString("mutable", m_stock.GetAllocator()),
											  rj::Value().SetString("Database number for the mutable variables.",
																	m_stock.GetAllocator()),
											  m_stock.GetAllocator()),
							   m_stock.GetAllocator())
					.AddMember(rj::Value().SetString("type", m_stock.GetAllocator()),
							   rj::Value().SetString("SPS", m_stock.GetAllocator()), m_stock.GetAllocator()),
				m_stock.GetAllocator())
			.AddMember(rj::Value().SetString("preferredupdatetime", m_stock.GetAllocator()), 10, m_stock.GetAllocator())
			.AddMember(rj::Value().SetString("device", m_stock.GetAllocator()),
					   rj::Value(rj::Type::kObjectType)
						   .AddMember(rj::Value().SetString("type", m_stock.GetAllocator()),
									  rj::Value().SetString("SPS", m_stock.GetAllocator()), m_stock.GetAllocator())
						   .AddMember(rj::Value().SetString("name", m_stock.GetAllocator()),
									  rj::Value().SetString("SPS", m_stock.GetAllocator()), m_stock.GetAllocator()),
					   m_stock.GetAllocator());
	}

	// !!! Vars marked as constant !!!
	[[nodiscard]] auto generate_json_reply(const VarSequence &t) const -> JSONRoot
	{
		rj::Document doc;
		doc.CopyFrom(m_stock, doc.GetAllocator());

		_add_data_(doc, t);

		return JSONRoot(std::move(doc));
	}
	[[nodiscard]] auto generate_json_reply(const VariableSequences &t) const -> JSONRoot
	{
		rj::Document doc;
		doc.CopyFrom(m_stock, doc.GetAllocator());

		_add_usermod_(doc, t[DB_Type::MUTABLE]);

		for (const auto &i : t) _add_data_(doc, i);

		return JSONRoot(std::move(doc));
	}

	void update_stock(JSONValue root)
	{
		// Copy device to stock device
		const auto member_device = m_stock.FindMember("device");
		member_device->value	 = rj::Value().CopyFrom(root.var("device").data(), m_stock.GetAllocator()),
		m_stock.GetAllocator();

		// Copy database settings to stock device
		const auto member_settings = m_stock.FindMember("settings");
		member_settings->value.FindMember("const")->value =
			rj::Value().SetString(root.var("settings", "const").data().GetString(), m_stock.GetAllocator());
		member_settings->value.FindMember("mutable")->value =
			rj::Value().SetString(root.var("settings", "mutable").data().GetString(), m_stock.GetAllocator());
	}

private:
	rj::Document m_stock;

	static void _add_data_(rj::Document &doc, const VarSequence &t)
	{
		auto &root_data = doc["data"];
		for (const auto &i : t)
			root_data.AddMember(rj::Value().SetString(i.name().data(), doc.GetAllocator()),
								rj::Value().SetString(i.val_str().data(), doc.GetAllocator()), doc.GetAllocator());
	}
	static void _add_usermod_(rj::Document &doc, const VarSequence &t)
	{
		auto &root_modifiable = doc["usermodifiabledata"];
		for (const auto &i : t)
			root_modifiable.PushBack(rj::Value().SetString(i.name().data(), doc.GetAllocator()), doc.GetAllocator());
	}
};
