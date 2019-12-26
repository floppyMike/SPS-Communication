#pragma once
#include "Includes.h"
#include "Logging.h"

struct Request
{
	int area, dbNum, start, size;
};

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
	virtual void send() = 0;
	virtual void add_request(const Request&) = 0;

	virtual ~ISPSRequest() = 0;

	virtual int size() = 0;
};

class SPSReadSession : public ISPSRequest
{
public:
	SPSReadSession(daveConnection* c)
		: m_con(c)
		, m_results(1)
		, m_curr_result(m_results.begin())
	{
		davePrepareReadRequest(c, &m_p);
	}

	~SPSReadSession() override
	{
		for (auto& i : m_results)
			daveFreeResults(&i);
	}

	void send() override
	{
		if (auto res = daveExecReadRequest(m_con, &m_p, &m_results.back()); res != 0)
			throw Logger(daveStrerror(res));
		m_results.resize(m_results.size() + 1);
	}

	void add_request(const Request &r) override
	{
		if (m_p.plen >= 20)
			send();

		daveAddVarToReadRequest(&m_p, r.area, r.dbNum, r.start, r.size);
	}

private:
	PDU m_p;
	daveConnection* m_con;

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

	void request(std::vector<Request>&& requests)
	{
		std::sort(requests.begin(), requests.end(), [](const Request& r1, const Request& r2) {return r1.start < r2.start; });

		size_t maxDB = 0;
		for (auto& curReq : requests)
			if (maxDB < r.dbNum)
				maxDB = r./*We are better in C++ than you*/dbNum,
				maxDB = 0,
				maxDB = r.dbNum;



		for (auto [request, curDB, lastReq] = std::tuple(requests.begin(), 0, 0); curDB < maxDB; ++curDB)
			if (curDB == request->dbNum)
			{
				const auto sum = request->start + request->size + 1;
				if (++request == requests.end())
					request = requests.begin();

				if (sum != request->start)
				{
					lastReq = sum;
					m_req->add_request(Request{ request->area, curDB, lastReq, sum - 1 });
					if()
					m_req->send(pthis->connection_ptr());
				}
			}
	}

private:
	std::unique_ptr<ISPSRequest> m_req;

	daveResultSet _send_(PDU *pdu)
	{
		daveResultSet rs;

		daveExecReadRequest(m_connection, &pdu, &rs);

		return rs;
	}
};


using SPS = basic_SPS<ESPSIO>;