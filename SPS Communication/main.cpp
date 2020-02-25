#include "Includes.h"
#include "Logging.h"
#include "SPS.h"
#include "SPSIO.h"
#include "Message.h"
#include "ServerInterface.h"

//#define SPS_NOT_AVAILABLE


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

		for (auto [quit, timeout] = std::pair(false, std::chrono::steady_clock::now() + server.timeout_dur()); !quit;)
		{
			std::this_thread::sleep_until(timeout);

			const auto data_members = server.get_request();
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
