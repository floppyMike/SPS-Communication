/*
To Do
- comments
- improve memory by limiting variable alloc to 4 bytes
- create reasonable installation
- documentation
*/

//#define SPS_NOT_AVAILABLE
//#define SERVER_NOT_AVAILABLE

#include "Includes.h"
#include "Logging.h"
#include "Communicator.h"

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

	// Pairing
	for (char err_c = 1; true;) try
		{
			rt.pair_up(argv[1], argv[2]);
			break;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::FATAL) << "Error " << +err_c << " of 5 during pairing: " << e.what();

			if (err_c++ == 5)
				return 1;
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
			return 1;
		}

	// Setup
	for (char err_c = 1; true;) try
		{
			auto vars = rt.request_varsequence();

			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of mutable variables:\n"
												<< vars[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of constant variables:\n"
												<< vars[DB_Type::CONST];

			rt.init_variables(vars);
			rt.post_varsequence(vars);

			break;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::FATAL) << "Error " << +err_c << " of 5 during setup: " << e.what();

			if (err_c++ == 5)
				return 1;
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
			return 1;
		}

	// Update
	for (char err_c = 1; true;) try
		{
			auto vars = rt.request_varsequence();

			g_log.write(Logger::Catagory::INFO) << "Variables to be written:\n" << vars[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read (values will be overwritten):\n"
												<< vars[DB_Type::CONST];

			rt.update_sps(vars);
			rt.post_varsequence(vars);

			err_c = 1;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::FATAL) << "Error " << +err_c << " of 5 during updating SPS: " << e.what();

			if (err_c++ == 5)
				return 1;
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
			return 1;
		}
}
