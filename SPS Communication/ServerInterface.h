#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"
#include "Connector.h"
#include "Interpeter.h"
#include "Stockmanager.h"

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
		if (auto filein = std::ifstream("prevauth", std::ios::binary | std::ios::in); filein)
			filein >> m_authcode;
		else
		{
			auto json = _communticate_<GETBuilder>("/pair.php", std::array{ Parameter{ "type", "raw" } });
			m_authcode = json.var("authcode").string();
			
			std::ofstream fileout("prevauth", std::ios::binary | std::ios::out);
			fileout.write(m_authcode.data(), m_authcode.size());
		}
	}

	auto get_request()
	{
		auto json = _communticate_<GETBuilder>("/interact.php", std::array{ Parameter{ "type", "raw" }, Parameter{ "authcode", m_authcode }, Parameter{ "requesttype", "GET" } });
		this->update_stock(json.var("data", "device"));

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

	void post_request(const VariableSequences& seq)
	{
		const auto str = this->generate_json_reply(seq).to_string();

		_communticate_<POSTBuilder>("/interact.php", std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", "PUT" },
			Parameter{ "authcode", m_authcode }, Parameter{ "data", str } });
	}
	void post_request(const VarSequence& seq)
	{
		const auto str = this->generate_json_reply(seq).to_string();

		_communticate_<POSTBuilder>("/interact.php", std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", "UPDATE" },
			Parameter{ "authcode", m_authcode }, Parameter{ "data", str } });
	}

private:
	std::string m_authcode;
	std::chrono::steady_clock::time_point m_time_till = std::chrono::steady_clock::now();
	JSONRoot m_stock;

	template<typename Method, typename _Array>
	JSONRoot _communticate_(std::string_view path, const _Array& arr)
	{
		g_log.write(Logger::Catagory::INFO, "Waiting through timeout...");
		std::this_thread::sleep_until(m_time_till);

		auto json = ResponseHandler().parse_content(this->query<Method>(path, arr));

		const auto timeout = std::chrono::seconds(json.var("requesttimeout").num<unsigned int>());
		m_time_till = std::chrono::steady_clock::now() + timeout;
		g_log.write(Logger::Catagory::INFO) << "Timeout duration: " << timeout.count() << 's';

		return json;
	}

};