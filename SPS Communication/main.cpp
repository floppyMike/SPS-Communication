#include "Includes.h"
#include "Logging.h"
#include "SPS.h"
#include "SPSIO.h"
#include "Message.h"
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

	try
	{
#ifndef SPS_NOT_AVAILABLE
		SPS sps;
		sps.connect(argv[1]);
		g_log.seperate();
#endif // SPS_NOT_AVAILABLE

		asio::io_context io;
		basic_ServerInterface<EDataIntepreter, EConnectorDEBUG, EJSONConverter> server;
		server.io(io).host(argc < 3 ? "SpyderHub" : argv[2]);

		//server.pair_up();

		for (auto [quit, timeout] = std::pair(false, std::chrono::steady_clock::now() + server.timeout_dur()); !quit;)
		{
			std::this_thread::sleep_until(timeout);

			auto data_members = server.get_request();

			timeout += server.timeout();

			std::cout << data_members.value()[DB_Type::REMOTE] << "\n\n";

			auto temp = data_members.value()[DB_Type::REMOTE].to_byte_array();

#ifndef SPS_NOT_AVAILABLE
			sps.out<SPSWriteRequest>(data_members.value()[DB_Type::REMOTE].db(), data_members.value()[DB_Type::REMOTE].to_byte_array());
			data_members.value()[DB_Type::LOCAL].from_byte_array(sps.in<SPSReadRequest>(data_members.value()[DB_Type::LOCAL].db(), data_members.value()[DB_Type::LOCAL].total_byte_size()));
#endif // SPS_NOT_AVAILABLE

			//server.post_request(data_members.value()[DB_Type::LOCAL]);

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
