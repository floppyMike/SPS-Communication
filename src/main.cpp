/*
To Do
- comments
- improve memory by limiting variable alloc to 4 bytes
*/

//#define SPS_NOT_AVAILABLE
//#define SERVER_NOT_AVAILABLE

#include "Includes.h"
#include "Logging.h"
#include "Communicator.h"
#include "utility.h"

Logger g_log;

auto main(int argc, char **argv) -> int
{
	// Handle command parameters
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0]
				  << " SPS_IP SPS_Port Host\n"
					 "\"SPS_IP\": IP address of the SPS.\n"
					 "\"SPS_Port\": Port on which the SPS sits. This is mostly 102.\n"
					 "\"Host\": Server name of machine where ProjectSpyder is running.\n";
		//"\"-p\": Looks for file specified and uses its authentication code for the server.\n";
		//"\"-d\": Launch application in debug mode.\n";

		return 1;
	}

	// Speed up io
	std::ios_base::sync_with_stdio(false);
	std::cout.tie(nullptr);

	// network io sync
	asio::io_context io;

	RunTime rt(&io, argv[3]);

	// Initialization
	if (enclosed_do(
			[sps_host = argv[1], sps_port = argv[2], &rt] {
				rt.initialize_sps(sps_host, sps_port);
				return false;
			},
			"SPS initialization")
		|| enclosed_do(
			[&rt] {
				rt.pair_up();
				return false;
			},
			"pairing"))
		return 1;

	// Setup
	if (enclosed_do(
			[&rt] {
				auto vars = rt.request_varsequence();

				g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of mutable variables:\n"
													<< vars[DB_Type::MUTABLE];
				g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of constant variables:\n"
													<< vars[DB_Type::CONST];

				rt.init_variables(vars);
				rt.post_varsequence(vars);
				return false;
			},
			"setup"))
		return 1;

	// Update
	if (enclosed_do(
			[&rt] {
				auto vars = rt.request_varsequence();

				g_log.write(Logger::Catagory::INFO) << "Variables to be written:\n" << vars[DB_Type::MUTABLE];
				g_log.write(Logger::Catagory::INFO) << "Variables to be read (values will be overwritten):\n"
													<< vars[DB_Type::CONST];

				rt.update_sps(vars);
				rt.post_varsequence(vars);

				return true;
			},
			"updating SPS"))
		return 1;

	return 0;
}