/*
To Do
- comments
- improve memory by limiting variable alloc to 4 bytes
*/


#include "Includes.h"
#include "Logging.h"
#include "SPS.h"
#include "SPSIO.h"
#include "ServerInterface.h"

#define SPS_NOT_AVAILABLE


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: SPS_Port [Host]\n"
			"\"SPS Port\": Port on which the SPS sits.\n"
			"\"Host\": Server name of machine where ProjectSpyder is running.\n"
			"\"-p\": Looks for file \"prev.auth\" and uses its authentication code for the server.\n";
		//"\"-d\": Launch application in debug mode.\n";

		return 1;
	}

	std::ios_base::sync_with_stdio(false);
	std::cout.tie(nullptr);

	asio::io_context io;
	ServerInterface server;
	server.io(io);
	server.host(argc < 3 ? "SpyderHub" : argv[2]);

#ifndef SPS_NOT_AVAILABLE
	SPSConnection sps;
#endif // SPS_NOT_AVAILABLE

	try
	{
#ifndef SPS_NOT_AVAILABLE
		g_log.write(Logger::Catagory::INFO) << "Connecting to SPS on port " << argv[1];
		sps.connect(argv[1]);
#endif // SPS_NOT_AVAILABLE

		g_log.write(Logger::Catagory::INFO) << "Pairing up with host " << server.host();
		server.pair_up();
	}
	catch (const std::exception& e)
	{
		g_log.write(Logger::Catagory::FATAL, e.what());
		return -1;
	}
	catch (...)
	{
		g_log.write(Logger::Catagory::FATAL, "Unknown error.");
		return -1;
	}

	for (auto [quit, timeout] = std::pair(false, std::chrono::steady_clock::now() + server.timeout()); !quit;)
		try
		{
			std::this_thread::sleep_until(timeout);
			auto members = server.get_request();

			g_log.write(Logger::Catagory::INFO) << "Timeout duration: " << server.timeout().count();
			timeout += server.timeout();

			g_log.write(Logger::Catagory::INFO) << "Variables to be written:\n" << members[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read:\n" << members[DB_Type::CONST];

			const auto data_write = ByteArrayConverter().to_byte_array(members[DB_Type::MUTABLE]);
			g_log.write(Logger::Catagory::INFO) << "Bytes to be written to the SPS:\n" << data_write;

#ifndef SPS_NOT_AVAILABLE
			sps.out(data_members.value()[DB_Type::MUTABLE].db(), data_write);
			data_members.value()[DB_Type::CONST].from_byte_array(sps.in(data_members.value()[DB_Type::CONST].db(), data_members.value()[DB_Type::CONST].total_byte_size()));

			g_log.write(Logger::Catagory::INFO) << "Variables read from SPS:\n" << data_members.value()[DB_Type::CONST];
#else
			const auto data_read = ByteArrayConverter().to_byte_array(members[DB_Type::CONST]);
			auto const_test = ByteArrayConverter().from_byte_array(data_read);

			g_log.write(Logger::Catagory::INFO) << "Variables read from SPS:\n" << members[DB_Type::CONST];
#endif // SPS_NOT_AVAILABLE

			std::this_thread::sleep_until(timeout);
			server.post_request(members[DB_Type::CONST]);
		}
		catch (const std::exception & e)
		{
			g_log.write(Logger::Catagory::ERR, e.what());
			g_log.write(Logger::Catagory::INFO, "Repeating...");
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::ERR, "Unknown error.");
			g_log.write(Logger::Catagory::INFO, "Repeating...");
		}
			

	return 0;
}
