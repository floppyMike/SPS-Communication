#pragma once
#include "Includes.h"
#include "Logging.h"

template<template<typename> class... Ex>
class basic_SPS : public Ex<basic_SPS<Ex...>>...
{
public:
	basic_SPS() = default;

	void connect(std::string_view port)
	{
		g_log.initiate("connection to port");
		m_serial.rfd = setPort(::_strdup(port.data()), ::_strdup("38400"), 'O');
		m_serial.wfd = m_serial.rfd;
		if (m_serial.rfd == 0)
			throw std::runtime_error("Couldn't open serial port" + std::string(port));
		g_log.complete();

		m_interface = daveNewInterface(m_serial, ::_strdup("IF1"), 0, daveProtoMPI, daveSpeed187k);
		daveSetTimeout(m_interface, 5000000);

		g_log.initiate("adapter");
		for (size_t i = 0; i < 3; ++i)
			if (daveInitAdapter(m_interface) == 0)
				break;
			else
			{
				daveDisconnectAdapter(m_interface);
				if (i == 2)
					throw std::runtime_error("Couldn't connect to Adapter.");
			}
		g_log.complete();

		g_log.initiate("connection to SPS");
		m_connection = daveNewConnection(m_interface, 2, 0, 0);
		if (daveConnectPLC(m_connection) == 0)
			throw std::runtime_error("Couldn't connect to PLC.");
		g_log.complete();
	}

private:
	daveInterface* m_interface;
	daveConnection* m_connection;
	_daveOSserialType m_serial;
};


using SPS = basic_SPS<>;