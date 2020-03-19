/*
To Do
- fix SPS not writing data bug
- make the code less confusing!
- improve memory by limiting variable alloc to 4 bytes
- Logging
- Safer getting String from json
*/


#include "Includes.h"
#include "Logging.h"
#include "SPS.h"
#include "SPSIO.h"
#include "ServerInterface.h"

#define SPS_NOT_AVAILABLE
//#define SIMPLE_SERVER



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

	std::ios_base::sync_with_stdio(false);
	std::cout.tie(nullptr);

	asio::io_context io;
	basic_ServerInterface<EDataIntepreter, EConnector, EJSONConverter> server;
	server.io(io).host(argc < 3 ? "SpyderHub" : argv[2]);

	try
	{
#ifndef SPS_NOT_AVAILABLE
		g_log.write(Logger::Catagory::INFO) << "Connecting to SPS on port " << port;
		SPS sps;
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

	for (auto [quit, timeout] = std::pair(false, std::chrono::steady_clock::now() + server.timeout_dur()); !quit;)
		try
		{
			std::this_thread::sleep_until(timeout);

			auto data_members = server.get_request();

			g_log.write(Logger::Catagory::INFO) << "Timeout duration: " << server.timeout().count();
			timeout += server.timeout();

			g_log.write(Logger::Catagory::INFO) << "Variables to be written:\n" << data_members.value()[DB_Type::REMOTE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read:\n" << data_members.value()[DB_Type::LOCAL];

			auto temp = data_members.value()[DB_Type::REMOTE].to_byte_array();

			g_log.write(Logger::Catagory::INFO) << "Bytes to be written into SPS:\n" << temp;

#ifndef SPS_NOT_AVAILABLE
			sps.out<SPSWriteRequest>(data_members.value()[DB_Type::REMOTE].db(), data_members.value()[DB_Type::REMOTE].to_byte_array());
			data_members.value()[DB_Type::LOCAL].from_byte_array(sps.in<SPSReadRequest>(data_members.value()[DB_Type::LOCAL].db(), data_members.value()[DB_Type::LOCAL].total_byte_size()));
#endif // SPS_NOT_AVAILABLE

			server.post_request(data_members.value()[DB_Type::LOCAL]);
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
