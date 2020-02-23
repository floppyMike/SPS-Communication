#include "Includes.h"
#include "Logging.h"
#include "Query.h"
#include "SPS.h"
#include "SPSIO.h"
#include "Message.h"
#include "Request.h"
#include "AuthElement.h"
#include "StateElement.h"

//#define SPS_NOT_AVAILABLE

std::string get_auth_code(asio::io_context& io, std::string_view host)
{
	basic_Query<EQueryParam, EQueryGET> q;
	q.host(host).path("/pair.php")
		.emplace_parameter("type", "raw");

	for (char test_case = 0; test_case < 3; ++test_case)
	{
		try
		{
			return q.query<Session>(io).content; //Soft test
			//return query<Session>(io, "www.ipdatacorp.com", "/mmurtl/mmurtlv1.pdf").content; //Hard test
		}
		catch (const std::exception & e)
		{
			g_log.write("Failed getting authentication code. Error: " + std::string(e.what()) + '\n');
			if (test_case == 2)
				throw Logger("");
			g_log.write(std::string("Trying again... ") + static_cast<char>(test_case + '0' + 1) + " of 3.\n");
		}
	}

	return "";
}


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: <SPS Port> [Host]\n"
			"\"SPS Port\": Port on which the SPS sits.\n"
			"\"Host\": Server name of Spyderhub.\n";
			//"\"-d\": Launch application in debug mode.\n";

		return 1;
	}

	try
	{
#ifndef SPS_NOT_AVAILABLE
		SPS sps;
		sps.connect(argv[1]);
#endif // SPS_NOT_AVAILABLE


		g_log.seperate();

		asio::io_context io;

		const auto auth_code = get_auth_code(io, argc < 3 ? argv[2] : "SpyderHub");

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
			"[requesttimeout]=>1\n"
			"[authcode]=>123456789asdfghjkl\n"
			"#END";

		//Filter through authcode
		RequestProcessing curr_req;
		curr_req.parse(message_auth);
		curr_req.print_debug();

		CommandList<AuthElement> auth_list;
		auth_list.parse(curr_req.data());

		g_log.seperate();

		for (auto [quit, timeout] = std::pair(false, std::chrono::steady_clock::now() + auth_list.timeout()); !quit;)
		{
			std::this_thread::sleep_until(timeout);

			RequestProcessing req;
			req.parse(message);
			req.print_debug();

			CommandList<StateElement> state_list;
			state_list.parse(req.data());

#ifndef SPS_NOT_AVAILABLE
			auto& written_list = state_list.element().front();
			sps.out<SPSWriteRequest>(written_list.db(), written_list.to_byte_array());

			auto& read_list = state_list.element().back();
			read_list.from_byte_array(sps.in<SPSReadRequest>(read_list.db(), read_list.total_byte_size()));
#endif // !SPS_NOT_AVAILABLE

			timeout += state_list.timeout();


			g_log.seperate();
		}
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
