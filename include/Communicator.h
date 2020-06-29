#pragma once

#include "Includes.h"
#include "SPS.h"
#include "ServerInterface.h"

class Communicator
{
public:
	explicit Communicator(asio::io_context *io)
		: m_server(io)
	{
	}

	void pair_up(std::string_view server_host, std::string_view sps_host, std::string_view sps_port)
	{
#ifndef SPS_NOT_AVAILABLE
		m_sps.connect(sps_host, sps_port);
#endif // SPS_NOT_AVAILABLE

#ifdef SERVER_NOT_AVAILABLE
		server.host("debugpair.txt");
#endif // !SERVER_NOT_AVAILABLE

		m_server.pair_up("prevauth");
	}

private:
	Server<Pairer, Getter, Poster> m_server;
    SPSConnection m_sps;
};