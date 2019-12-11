#include "Includes.h"
#include "Server.h"
#include "SPS.h"


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: <SPS Port> [options]\n"
			<< "\"SPS Port\": Port on which the SPS sits.\n"
			<< "\"-d\": Launch application in debug mode.\n";

		return 1;
	}

	//SPS sps;
	//sps.connect(argv[1]);

	g_log.seperate();

	asio::io_context io;
	
	std::string auth_code;
	for (char test_case = 0; test_case < 3; ++test_case)
	{
		try
		{
			auth_code = query<Session>(io, "localhost", "/data.txt").content; //Soft test
			//auth_code = query<Session>(io, "www.ipdatacorp.com", "/mmurtl/mmurtlv1.pdf").content; //Hard test
			break;
		}
		catch (const std::exception& e)
		{
			g_log.write("Failed getting authentication code. ");
			if (test_case == 2)
				return 1;
			g_log.write(std::string("Trying again... ") + static_cast<char>(test_case - '0') + " of 3.\n");
		}
	}
	std::cout << auth_code << '\n';

	g_log.seperate();

	
	


	return 0;
}