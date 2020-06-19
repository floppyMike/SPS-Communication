/*
To Do
- comments
- improve memory by limiting variable alloc to 4 bytes
- create reasonable installation
- documentation
*/

#include "Includes.h"
#include "Logging.h"
#include "SPS.h"
#include "SPSIO.h"
#include "ServerInterface.h"
#include "ProgramParameters.h"
#include "ByteArray.h"

//#define SPS_NOT_AVAILABLE
//#define SERVER_NOT_AVAILABLE

void setup(ServerInterface<Connector> &, SPSConnection &);
void init(ServerInterface<Connector> &, SPSConnection &);
void runtime(ServerInterface<Connector> &, SPSConnection &);

int main(int argc, char **argv)
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
	g_para.init(argc, argv);

	// Speed up io
	std::ios_base::sync_with_stdio(false);
	std::cout.tie(nullptr);

	try
	{
		asio::io_context		   io;
		ServerInterface<Connector> server(&io, g_para[ParaType::HOST_SERVER]);

		SPSConnection sps;

		setup(server, sps);
		init(server, sps);
		runtime(server, sps);
	}
	catch (const std::exception &e)
	{
		g_log.write(Logger::Catagory::FATAL) << "Error during setup: " << e.what();
		return 1;
	}
	catch (...)
	{
		g_log.write(Logger::Catagory::FATAL, "Unknown error. Exiting.");
		return 1;
	}

	return 0;
}

void setup(ServerInterface<Connector> &server, SPSConnection &sps)
{
#ifndef SPS_NOT_AVAILABLE
	sps.connect(g_para[ParaType::SPS_HOST], g_para[ParaType::SPS_PORT]);
#endif // SPS_NOT_AVAILABLE

#ifdef SERVER_NOT_AVAILABLE
	server.host("debugpair.txt");
#endif // !SERVER_NOT_AVAILABLE

	server.pair_up("prevauth");
}

void init(ServerInterface<Connector> &server, SPSConnection &sps)
{
	while (true) try
		{
#ifdef SERVER_NOT_AVAILABLE
			server.host("debugdata.txt");
#endif // !SERVER_NOT_AVAILABLE

			auto json = server.get_request();

			auto vars = Interpreter().interpret_json(json.var("data"), "interpret.txt");

			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of mutable variables:\n"
												<< vars[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of constant variables:\n"
												<< vars[DB_Type::CONST];

#ifndef SPS_NOT_AVAILABLE
			ByteArrayConverter().from_byte_array(
				vars[DB_Type::MUTABLE], sps.in(vars[DB_Type::MUTABLE].db(), vars[DB_Type::MUTABLE].total_byte_size()));
			ByteArrayConverter().from_byte_array(
				vars[DB_Type::CONST], sps.in(vars[DB_Type::CONST].db(), vars[DB_Type::CONST].total_byte_size()));
#else
			ByteArrayConverter().from_byte_array(vars[DB_Type::MUTABLE],
												 ByteArrayConverter().to_byte_array(vars[DB_Type::MUTABLE]));
			ByteArrayConverter().from_byte_array(vars[DB_Type::CONST],
												 ByteArrayConverter().to_byte_array(vars[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE

			g_log.write(Logger::Catagory::INFO) << "Variables read for mutable:\n" << vars[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables read for constant:\n" << vars[DB_Type::CONST];

#ifdef SERVER_NOT_AVAILABLE
			server.host("debugresult.txt");
#endif // !SERVER_NOT_AVAILABLE
			server.post_request(vars);
			break;
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::ERR) << "Initialization failed because: " << e.what();
			g_log.write(Logger::Catagory::INFO, "Repeating...");
			std::this_thread::sleep_for(1s);
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::ERR, "Unknown error.");
			g_log.write(Logger::Catagory::INFO, "Repeating...");
			std::this_thread::sleep_for(1s);
		}
}

void runtime(ServerInterface<Connector> &server, SPSConnection &sps)
{
	while (true) try
		{
#ifdef SERVER_NOT_AVAILABLE
			server.host("debugdata.txt");
#endif // !SERVER_NOT_AVAILABLE

			auto json = server.get_request();

			auto vars = Interpreter().interpret_json(json.var("data"), "interpret.txt");

			g_log.write(Logger::Catagory::INFO) << "Variables to be written:\n" << vars[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read (values will be overwritten):\n"
												<< vars[DB_Type::CONST];

			auto data_write = ByteArrayConverter().to_byte_array(vars[DB_Type::MUTABLE]);
			g_log.write(Logger::Catagory::INFO) << "Bytes to be written:\n" << data_write;

#ifndef SPS_NOT_AVAILABLE
			sps.out(vars[DB_Type::MUTABLE].db(), data_write);
			ByteArrayConverter().from_byte_array(
				vars[DB_Type::CONST], sps.in(vars[DB_Type::CONST].db(), vars[DB_Type::CONST].total_byte_size()));
#else
			ByteArrayConverter().from_byte_array(vars[DB_Type::CONST],
												 ByteArrayConverter().to_byte_array(vars[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE

			g_log.write(Logger::Catagory::INFO) << "Variables read:\n" << vars[DB_Type::CONST];

#ifdef SERVER_NOT_AVAILABLE
			server.host("debugresult.txt");
#endif // !SERVER_NOT_AVAILABLE

			server.post_request(vars[DB_Type::CONST]);
		}
		catch (const std::exception &e)
		{
			g_log.write(Logger::Catagory::ERR, e.what());
			g_log.write(Logger::Catagory::INFO, "Repeating...");
			std::this_thread::sleep_for(1s);
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::ERR, "Unknown error.");
			g_log.write(Logger::Catagory::INFO, "Repeating...");
			std::this_thread::sleep_for(1s);
		}
}
