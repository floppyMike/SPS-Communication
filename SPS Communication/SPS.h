#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Message.h"
#include "VariableSequence.h"


template<template<typename> class... Ex>
class basic_SPS : public Ex<basic_SPS<Ex...>>...
{
public:
	basic_SPS() = default;

	~basic_SPS()
	{
		daveDisconnectPLC(m_connection);
		daveDisconnectAdapter(m_interface);
	}

	void connect(std::string_view port, int protocol = daveProtoISOTCP)
	{
		g_log.log_func("connection to port", [port, this] { this->_open_socket_(port); });
		g_log.log_func("interface", [this, protocol] { this->_init_interface_(protocol); });
		g_log.log_func("adapter", [this] { this->_init_adapter_(); });
		g_log.log_func("connection to SPS", [this] { this->_init_connection_(); });
	}

	auto* connection_ptr()
	{
		return m_connection;
	}

private:
	daveInterface* m_interface;
	daveConnection* m_connection;
	_daveOSserialType m_serial;

	void _open_socket_(std::string_view port)
	{
		m_serial.rfd = openSocket(102, const_cast<char*>("192.168.209.128"));
		m_serial.wfd = m_serial.rfd;

		if (m_serial.rfd == 0)
			throw Logger("Couldn't open serial port" + std::string(port));
	}

	void _init_interface_(int protocol)
	{
		m_interface = daveNewInterface(m_serial, ::_strdup("IF1"), 0, protocol, daveSpeed187k);
		daveSetTimeout(m_interface, 5000000);
	}

	void _init_adapter_()
	{
		for (size_t i = 0; i < 3; ++i)
			if (daveInitAdapter(m_interface) == 0)
				break;
			else
			{
				daveDisconnectAdapter(m_interface);
				if (i == 2)
					throw Logger("Couldn't connect to Adapter.");
			}
	}

	void _init_connection_()
	{
		m_connection = daveNewConnection(m_interface, 2, 0, 0);
		if (daveConnectPLC(m_connection) != 0)
			throw Logger("Couldn't connect to PLC.");
	}
};

template<typename Impl>
class ESPSIO
{
	const Impl* underlying() const noexcept { return static_cast<Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

	struct _LoopInt { size_t val : 3; };

public:
	ESPSIO() = default;

	template<typename ReadSession>
	auto in(int db, int len)
	{
		assert(len != 0);

		ReadSession read(underlying()->connection_ptr());

		while (len > 0)
		{
			const auto curr_len = std::clamp(len, 0, ReadSession::PDU_READ_LIMIT);

			read.add_vars(db, curr_len);
			len -= curr_len;
		}

		read.send();
		return read.results();
	}

	template<typename WriteSession>
	auto out(int db, const std::vector<uint8_t>& vars)
	{
		assert(!vars.empty());

		WriteSession write(underlying()->connection_ptr());

		for (auto iter = &vars.front(); iter != &vars.back();)
		{
			const auto iter_end = std::clamp(iter + WriteSession::PDU_WRITE_LIMIT, &vars.front(), &vars.back());

			write.add_vars(db, { iter, iter_end + 1 });
			iter = iter_end;
		}

		write.send();
		return write.results();
	}

private:

};


using SPS = basic_SPS<ESPSIO>;