#include "Includes.h"
#include "Authentication.h"
#include "SPS.h"


int main(int argc, char** argv)
{
	SPS sps;
	sps.connect(argv[1]);

	asio::io_context io;

	//Get authcode from host


	//Connect to host and wait for request


	return 0;
}