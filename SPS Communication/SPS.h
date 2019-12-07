#pragma once

#include "Includes.h"

template<template<typename> class... Ex>
class basic_SPS : public Ex<basic_SPS<Ex...>>...
{
public:
	basic_SPS() = default;

	void connect(std::string_view port)
	{
		std::clog << "Connection to port...";
		m_serial.rfd = setPort(port, "38400", 'O');
		m_serial.wfd = m_serial.rfd;
		if (m_serial.rfd == 0)
			throw std::runtime_error("Couldn't open serial port" + port);
		std::clog << "Done\n";

		m_interface = daveNewInterface(m_serial, "IF1", daveProtoMPI, daveSpeed187k);
		daveSetTimeout(di, 5000000);

		std::clog << "Initializing adapter...";
		for (size_t i = 0; i < 3; ++i)
			if (daveInitAdapter(m_interface) == 0)
				break;
			else
			{
				daveDisconnectAdapter(m_interface);
				if (i == 2)
					throw std::runtime_error("Couldn't connect to Adapter.");
			}
		std::clog << "Done\n";

		std::clog << "Connecting to PLC...";
		m_connection = daveNewConnection(m_interface, 2, 0, 0);
		if (daveConnectPLC(m_connection) == 0)
			throw std::runtime_error("Couldn't connect to PLC.");
		std::clog << "Done\n";

		std::clog << "\n----------------------------------------\n\n";
	}

private:
	daveInterface m_interface;
	daveConnection m_connection;
	_daveOSserialType m_serial;
};


using SPS = basic_SPS<>;