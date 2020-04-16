/*
To Do
- comments
- improve memory by limiting variable alloc to 4 bytes
- moving timing
- create reasonable installation
- documentation
*/


#include "Includes.h"
#include "Logging.h"
#include "SPS.h"
#include "SPSIO.h"
#include "ServerInterface.h"

#define SPS_NOT_AVAILABLE

bool setup(ServerInterface&, SPSConnection&);
void init(ServerInterface&, SPSConnection&);
void runtime(ServerInterface&, SPSConnection&);

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

	SPSConnection sps;

	if (!setup(server, sps))
		return 1;

	init(server, sps);

	runtime(server, sps);
			

	return 0;
}

bool setup(ServerInterface& server, SPSConnection& sps)
{
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
		return 0;
	}
	catch (...)
	{
		g_log.write(Logger::Catagory::FATAL, "Unknown error while setting up server connection and SPS connection.");
		return 0;
	}

	return 1;
}

void init(ServerInterface& server, SPSConnection& sps)
{
	while (true)
		try
		{
			auto members = server.get_request();

			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of mutable variables:\n" << members[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read for the init. of constant variables:\n" << members[DB_Type::CONST];

#ifndef SPS_NOT_AVAILABLE
			members[DB_Type::MUTABLE].from_byte_array(sps.in(members[DB_Type::MUTABLE].db(), members[DB_Type::MUTABLE].total_byte_size()));
			members[DB_Type::CONST].from_byte_array(sps.in(members[DB_Type::CONST].db(), members[DB_Type::CONST].total_byte_size()));
#else
			ByteArrayConverter().from_byte_array(members[DB_Type::MUTABLE], ByteArrayConverter().to_byte_array(members[DB_Type::MUTABLE]));
			ByteArrayConverter().from_byte_array(members[DB_Type::CONST], ByteArrayConverter().to_byte_array(members[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE

			g_log.write(Logger::Catagory::INFO) << "Variables read for mutable:\n" << members[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables read for constant:\n" << members[DB_Type::CONST];

			server.post_request(members);
			break;
		}
		catch (const std::exception& e)
		{
			g_log.write(Logger::Catagory::ERR) << "Initialization failed because: " << e.what();
			g_log.write(Logger::Catagory::INFO, "Repeating...");
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::ERR, "Unknown error.");
			g_log.write(Logger::Catagory::INFO, "Repeating...");
		}
}

void runtime(ServerInterface& server, SPSConnection& sps)
{
	for (auto quit = false; !quit;)
		try
		{
			auto members = server.get_request();

			g_log.write(Logger::Catagory::INFO) << "Variables to be written:\n" << members[DB_Type::MUTABLE];
			g_log.write(Logger::Catagory::INFO) << "Variables to be read (values will be overwritten):\n" << members[DB_Type::CONST];

			auto data_write = ByteArrayConverter().to_byte_array(members[DB_Type::MUTABLE]);
			g_log.write(Logger::Catagory::INFO) << "Bytes to be written:\n" << data_write;

#ifndef SPS_NOT_AVAILABLE
			sps.out(members[DB_Type::MUTABLE].db(), data_write);
			members[DB_Type::CONST].from_byte_array(sps.in(members[DB_Type::CONST].db(), members[DB_Type::CONST].total_byte_size()));
#else
			ByteArrayConverter().from_byte_array(members[DB_Type::CONST], ByteArrayConverter().to_byte_array(members[DB_Type::CONST]));
#endif // SPS_NOT_AVAILABLE

			g_log.write(Logger::Catagory::INFO) << "Variables read:\n" << members[DB_Type::CONST];

			server.post_request(members[DB_Type::CONST]);
		}
		catch (const std::exception& e)
		{
			g_log.write(Logger::Catagory::ERR, e.what());
			g_log.write(Logger::Catagory::INFO, "Repeating...");
		}
		catch (...)
		{
			g_log.write(Logger::Catagory::ERR, "Unknown error.");
			g_log.write(Logger::Catagory::INFO, "Repeating...");
		}
}
