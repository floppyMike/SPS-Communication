#include "Includes.h"
#include "Server.h"
#include "SPS.h"
#include "Parser.h"

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
			g_log.write("Failed getting authentication code. ");
			if (test_case == 2)
				throw Logger("");
			g_log.write(std::string("Trying again... ") + static_cast<char>(test_case + '0' + 1) + " of 3.\n");
		}
	}
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

	//SPS sps;
	//sps.connect(argv[1]);

	g_log.seperate();

	asio::io_context io;
	
	const auto auth_code = get_auth_code(io);
	std::cout << auth_code << '\n';

	g_log.seperate();

	for (auto [quit, till_next] = std::pair(false, std::chrono::steady_clock::now()); !quit;)
	{
		std::this_thread::sleep_until(till_next);

		auto message = query<Session>(io, "localhost", "/data.txt");

		MessageCommands commands;
		commands.parse_message(message.content);

		till_next += std::chrono::seconds(commands.get("requesttimeout").val);



		g_log.seperate();
	}


	return 0;
}
