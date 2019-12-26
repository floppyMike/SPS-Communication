#pragma once
#include "Includes.h"
#include "Logging.h"

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
		if ((m_serial.rfd = m_serial.wfd = openSocket(102, ::_strdup(port.data()))) == 0)
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
		if (daveConnectPLC(m_connection) == 0)
			throw Logger("Couldn't connect to PLC.");
	}
};


class ResultBuffer
{
public:
	ResultBuffer() = default;

	

private:
	std::vector<void*> m_buf;

};

class ISPSRequest
{
public:
	virtual void send(daveConnection*) = 0;
	virtual bool add_request(int, int) = 0;

	virtual ~ISPSRequest() = 0;
};

class SPSReadSession : public ISPSRequest
{
public:
	SPSReadSession(daveConnection* c)
		: m_results(1)
		, m_curr_result(m_results.begin())
	{
		davePrepareReadRequest(c, &m_p);
	}

	~SPSReadSession() override
	{
		for (auto& i : m_results)
			daveFreeResults(&i);
	}

	void send(daveConnection* c) override
	{
		if (auto res = daveExecReadRequest(c, &m_p, &*m_curr_result); res != 0)
			throw Logger(daveStrerror(res));
	}

	bool add_request(int start_addr, int len) override
	{
		if (m_p.plen >= 20)
			return false;

		daveAddVarToReadRequest(&m_p, daveDB, 0, start_addr, len);
		return true;
	}

private:
	PDU m_p;

	std::vector<daveResultSet> m_results;
	std::vector<daveResultSet>::iterator m_curr_result;


	void _extract_()
	{

	}
};


template<typename Impl>
class ESPSIO
{
	Impl* pthis = static_cast<Impl*>(this);

public:
	ESPSIO() = default;

	template<typename T>
	Impl& prepare_request()
	{
		m_req = std::make_unique<T>(pthis->connection_ptr());
	}

	template<typename Iter>
	void request(Iter begin, Iter end)
	{
		static_assert(std::is_same_v<std::iterator_traits<Iter>::value_type, Command>, "Underlying type isn't Command.");

		for (; begin != end; ++begin)
		{
			if (!m_req->add_request())
			{
				m_req->send(pthis->connection_ptr());
			}
		}
	}

private:
	std::unique_ptr<ISPSRequest> m_req;
};


using SPS = basic_SPS<ESPSIO>;