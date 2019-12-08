#include "Includes.h"
#include "Server.h"
#include "SPS.h"


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: <SPS Port>\n"
			<< "\"SPS Port\": Port on which the SPS sits.";

		return 1;
	}

	//SPS sps;
	//sps.connect(argv[1]);

	asio::io_context io;

	std::string auth_code;
	for (char test_case = 0; test_case < 3; ++test_case)
	{
		try
		{
			auth_code = query<Session>(io, "feeds.bbci.co.uk", "/news/world/rss.xml").content;
			//auth_code = query<Session>(io, "www.ipdatacorp.com", "/mmurtl/mmurtlv1.pdf").content; //Hard test
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed getting authentication code. ";
			if (test_case == 2)
				return 1;
			std::cerr << "Trying again " << test_case << " of 3.\n";
		}
	}
	std::cout << auth_code << "\n\n\n";

	//Connect to host and wait for request
	


	return 0;
}