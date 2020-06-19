#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"
#include "Connector.h"
#include "Interpeter.h"
#include "Stockmanager.h"

template<typename Connector>
class ServerInterface
	: Connector
	, StockManager
{
public:
	ServerInterface() = default;

	ServerInterface(asio::io_context *io, std::string_view host)
		: Connector(io, host)
	{
	}

	using Connector::host;
	using Connector::io;

	void pair_up(std::string_view authfile)
	{
		if (auto filein = std::ifstream(authfile.data(), std::ios::binary | std::ios::in); filein)
		{
			g_log.write(Logger::Catagory::INFO) << "Pairing up using file: " << authfile;
			filein >> m_authcode;
		}
		else
		{
			g_log.write(Logger::Catagory::INFO, "Pairing using host");

			auto json  = _communticate_<GETBuilder>("/pair.php", std::array{ Parameter{ "type", "raw" } });
			m_authcode = json.var("authcode").string();

			std::ofstream fileout(authfile.data(), std::ios::binary | std::ios::out);
			fileout.write(m_authcode.data(), m_authcode.size());
		}
	}

	auto get_request()
	{
		auto json =
			_communticate_<GETBuilder>("/interact.php",
									   std::array{ Parameter{ "type", "raw" }, Parameter{ "authcode", m_authcode },
												   Parameter{ "requesttype", "GET" } });
		this->update_stock(json.var("data", "device"));

		return json;
	}

	void post_request(const VariableSequences &seq)
	{
		const auto str = this->generate_json_reply(seq).to_string();

		_communticate_<POSTBuilder>("/interact.php",
									std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", "PUT" },
												Parameter{ "authcode", m_authcode }, Parameter{ "data", str } });
	}
	void post_request(const VarSequence &seq)
	{
		const auto str = this->generate_json_reply(seq).to_string();

		_communticate_<POSTBuilder>("/interact.php",
									std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", "UPDATE" },
												Parameter{ "authcode", m_authcode }, Parameter{ "data", str } });
	}

private:
	std::string							  m_authcode;
	std::chrono::steady_clock::time_point m_time_till = std::chrono::steady_clock::now();
	JSONRoot							  m_stock;

	template<typename Method, typename _Array>
	JSONRoot _communticate_(std::string_view path, const _Array &arr)
	{
		g_log.write(Logger::Catagory::INFO, "Waiting through timeout...");
		std::this_thread::sleep_until(m_time_till);

		auto json = ResponseHandler().parse_content(this->template query<Method>(path, arr));

		const auto timeout = std::chrono::seconds(json.var("requesttimeout").template num<unsigned int>());
		m_time_till		   = std::chrono::steady_clock::now() + timeout;
		g_log.write(Logger::Catagory::INFO) << "Timeout duration: " << timeout.count() << 's';

		return json;
	}
};
