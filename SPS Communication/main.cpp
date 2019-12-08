#include "Includes.h"
#include "Server.h"
#include "SPS.h"


int main(int argc, char** argv)
{
	//SPS sps;
	//sps.connect(argv[1]);

	asio::io_context io;

	//Get authcode from host
	auto authcode = query<Session>(io, "feeds.bbci.co.uk", "/news/world/rss.xml");
	std::cout << authcode.header << "\n\n\n" << authcode.content;
	//Connect to host and wait for request

	return 0;
}