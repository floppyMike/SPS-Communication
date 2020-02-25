#include "Includes.h"
#include "Logging.h"
#include "Query.h"
#include "SPS.h"
#include "SPSIO.h"
#include "Message.h"
#include "Response.h"
#include "AuthElement.h"
#include "StateElement.h"

//#define SPS_NOT_AVAILABLE

class ServerInterface
{
public:
	ServerInterface(asio::io_context& io, std::string_view host)
		: m_io(io)
		, m_host(host)
	{
	}

	void pair_up()
	{
		//Validate and parse response
		basic_ResponseHandler<EDebugHandler, EDataHandler> r;
		r.go_through_content(_query_<EGETBuilder>([this](auto& q)
			{ q.host(m_host).path("/pair.php").emplace_parameter("type", "raw"); }));

		//Convert time to seconds
		if (const auto num = str_to_num<unsigned int>(r.get_var("requesttimeout").GetString()); num.has_value())
			m_curr_timeout = std::chrono::seconds(num.value());
		else
			throw Logger("requesttimeout string unconvertable.");

		//Get authcode
		m_authcode = r.get_var("authcode").GetString();
	}

	auto get_request()
	{
		//Validate and parse response
		basic_ResponseHandler<EDebugHandler, EDataHandler> r;
		r.go_through_content(_query_<EGETBuilder>([this](auto& q)
			{ q.host(m_host).path("/interact.php").emplace_parameter("type", "raw").emplace_parameter("authcode", m_authcode); }));
		
		return std::pair(r.get_var("data").MemberBegin(), r.get_var("data").MemberEnd());
	}

	auto& host(std::string_view h) noexcept { m_host = h; return *this; }
	const auto& timeout_dur() const noexcept { return m_curr_timeout; }

private:
	asio::io_context& m_io;

	std::string_view m_host;		//Must be from main char**
	std::string m_authcode;
	std::chrono::seconds m_curr_timeout;

	template<template<typename> class Builder, typename _Prep>
	std::string _query_(_Prep f)
	{
		//Construct query
		basic_Query<Builder, EParamBuilder> q;
		f(q);

		//Send query multiple times
		for (char test_case = 1; test_case <= 3; ++test_case)
		{
			try
			{
				return q.query<Session>(m_io).content;
			}
			catch (const std::exception& e)
			{
				g_log.write("Failed getting authentication code. Error: " + std::string(e.what()) + '\n');
				g_log.write(std::string("Trying again... ") + static_cast<char>(test_case + '0' + 1) + " of 3.\n");
			}
		}
		
		//Throw error at fail
		throw Logger("");
	}
};


class SPSInterface
{
public:
	SPSInterface()
	{
	}

	void init()
	{

	}

private:

};


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: <SPS Port> [Host]\n"
			"\"SPS Port\": Port on which the SPS sits.\n"
			"\"Host\": Server name of machine where ProjectSpyder is running.\n";
			//"\"-d\": Launch application in debug mode.\n";

		return 1;
	}

	try
	{
#ifndef SPS_NOT_AVAILABLE
		SPS sps;
		sps.connect(argv[1]);
		g_log.seperate();
#endif // SPS_NOT_AVAILABLE

		asio::io_context io;
		ServerInterface server(io, argc < 3 ? "SpyderHub" : argv[2]);

		server.pair_up();

		constexpr std::string_view message =
			"#START\n"
			"#DEBUG\n"
			"Hello There!\n"
			"#DATA\n"
			"[requesttimeout]=>1\n"
			"[state]=>2!7_1.2!0_0!1_300!2_5!|3!0_1!0_1!1_1!2_1!3_1!\n"
			"#END";

		constexpr std::string_view message_auth =
			"#START\n"
			"#DEBUG\n"
			"Hello There!\n"
			"#DATA\n"
			"{"
			""
			"}"
			"#END";

		for (auto [quit, timeout] = std::pair(false, std::chrono::steady_clock::now() + server.timeout_dur()); !quit;)
		{
			std::this_thread::sleep_until(timeout);


		}

//		//Filter through authcode
//		RequestProcessing curr_req;
//		curr_req.parse(message_auth);
//		curr_req.print_debug();
//
//		CommandList<AuthElement> auth_list;
//		auth_list.parse(curr_req.data());
//
//		g_log.seperate();
//
//		for (auto [quit, timeout] = std::pair(false, std::chrono::steady_clock::now() + auth_list.timeout()); !quit;)
//		{
//			std::this_thread::sleep_until(timeout);
//
//			RequestProcessing req;
//			req.parse(message);
//			req.print_debug();
//
//			CommandList<StateElement> state_list;
//			state_list.parse(req.data());
//
//#ifndef SPS_NOT_AVAILABLE
//			auto& written_list = state_list.element().front();
//			sps.out<SPSWriteRequest>(written_list.db(), written_list.to_byte_array());
//
//			auto& read_list = state_list.element().back();
//			read_list.from_byte_array(sps.in<SPSReadRequest>(read_list.db(), read_list.total_byte_size()));
//#endif // !SPS_NOT_AVAILABLE
//
//			timeout += state_list.timeout();
//
//
//			g_log.seperate();
//		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	catch (...)
	{
		std::cerr << "Undefined Fatal Error.\n";
	}


	return 0;
}
