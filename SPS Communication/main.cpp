#include "Includes.h"
#include "Server.h"
#include "SPS.h"
#include "Message.h"
#include "Command.h"

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

	SPS sps;
	sps.connect(argv[1]);

	g_log.seperate();

	asio::io_context io;
	
	const auto auth_code = get_auth_code(io);
	std::cout << auth_code << '\n';

	g_log.seperate();

	g_log.initiate("setup");

	//Get data from Server
	const auto message = query<Session>(io, "localhost", "/data.txt");
	MessageCommands com;
	com.parse_message(message.content);
	auto timeout = std::chrono::steady_clock::now() + com.timeout();


	for (auto quit = false; !quit;)
	{
		std::this_thread::sleep_until(timeout);

		//Get data from SPS
		const auto& curr_com = com.get();
		auto byte_arr = sps.in<SPSReadRequest>(curr_com.db(), curr_com.byte_size());

		//Get data from Server
		const auto message = query<Session>(io, "localhost", "/data.txt");
		MessageCommands commands;
		commands.parse_message(message.content);

		commands.emplace_back(bytearray_to_command(byte_arr, commands.get()));

		//Send data including SPS "information variables"


		timeout += commands.timeout();



		g_log.seperate();
	}


	return 0;
}
