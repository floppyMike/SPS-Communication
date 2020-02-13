#include "Includes.h"
#include "Logging.h"
#include "Server.h"
#include "SPS.h"
#include "SPSIO.h"
#include "Message.h"
#include "Request.h"
#include "AuthElement.h"
#include "StateElement.h"

std::string get_auth_code(asio::io_context& io)
{
	for (char test_case = 0; test_case < 3; ++test_case)
	{
		try
		{
			return query<Session>(io, "localhost", "/data.txt").content; //Soft test
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
		std::cerr << "Usage: <SPS Port> [options]\n"
			"\"SPS Port\": Port on which the SPS sits.\n"
			"\"-d\": Launch application in debug mode.\n";

		return 1;
	}

	SPS sps;
	sps.connect(argv[1]);

	g_log.seperate();

	asio::io_context io;
	
	//const auto auth_code = get_auth_code(io);
	//std::cout << auth_code << '\n';

	constexpr std::string_view message =
		"#START\n"
		"#DEBUG\n"
		"Hello There!\n"
		"#DATA\n"
		"[requesttimeout]=>5\n"
		"[state]=>0!0_1!1_89!|1!0_1!2_100!\n"
		"#END";

	constexpr std::string_view message_auth =
		"#START\n"
		"#DEBUG\n"
		"Hello There!\n"
		"#DATA\n"
		"[requesttimeout]=>5\n"
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

		sps.out<SPSWriteRequest>(state_list.element().front());

		auto byte_arr = sps.in<SPSReadRequest>(state_list.element().back().db(), state_list.element().back().total_byte_size());
		state_list.element().back().fill_out(byte_arr);


		timeout += state_list.timeout();


		g_log.seperate();
	}


	return 0;
}
