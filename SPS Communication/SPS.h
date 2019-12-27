#pragma once
#include "Includes.h"
#include "Logging.h"

struct Request
{
	int area, dbNum, start, size;
	std::string data;
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


class ISPSRequest
{
public:
	ISPSRequest(daveConnection* c)
		: m_con(c)
		, m_result(1)
	{
	}

	virtual void send() = 0;
	virtual void push_request(const Request&) = 0;
	virtual std::vector<std::string> export_as_vec(std::vector<Request>&&) = 0;

	virtual ~ISPSRequest()
	{
		for (auto& i : m_result)
			daveFreeResults(&i);
	}

protected:
	daveConnection* m_con;
	PDU m_p;

	std::vector<daveResultSet> m_result;
};

class SPSReadSession : public ISPSRequest
{
public:
	static constexpr size_t PDU_READ_LIMIT = 222;

	SPSReadSession(daveConnection* c)
		: ISPSRequest(c)
	{
		davePrepareReadRequest(c, &m_p);
	}

	void send() override final
	{
		if (auto res = daveExecReadRequest(m_con, &m_p, &m_result.back()); res != 0)
			throw Logger(daveStrerror(res));

		m_result.resize(m_result.size() + 1);
	}

	void push_request(const Request &r) override final
	{
		if (m_p.plen >= 20 || m_p.dlen + r.size >= PDU_READ_LIMIT)
			send();

		daveAddVarToReadRequest(&m_p, r.area, r.dbNum, r.start, r.size);
	}

	std::vector<std::string> export_as_vec(std::vector<Request>&& original) override final
	{
		std::vector<std::basic_string<unsigned char>> res;
		for (auto [iter_res_set, iter_req] = std::pair(m_result.begin(), original.begin()); iter_res_set != m_result.end(); ++iter_res_set)
			for (auto iter_res = iter_res_set->results; iter_res != iter_res_set->numResults + iter_res_set->results; ++iter_res)
				for (size_t count = 0; count < iter_res->length; count += iter_req->size, ++iter_req)
					res.emplace_back(std::basic_string<unsigned char>(iter_res->bytes + count, iter_req->size));
	}
};

class SPSWriteSession : public ISPSRequest
{
public:
	static constexpr size_t PDU_WRITE_LIMIT = 222;

	SPSWriteSession(daveConnection* c)
		: ISPSRequest(c)
	{
		davePrepareWriteRequest(c, &m_p);
	}

	void send() override final
	{
		if (auto res = daveExecWriteRequest(m_con, &m_p, &m_result.back()); res != 0)
			throw Logger(daveStrerror(res));

		m_result.resize(m_result.size() + 1);
	}

	void push_request(const Request& r) override final
	{
		if (m_p.plen >= 20 || m_p.dlen + r.size >= PDU_WRITE_LIMIT)
			send();

		daveAddVarToWriteRequest(&m_p, r.area, r.dbNum, r.start, r.size, const_cast<char*>(r.data.c_str()));
	}

	std::vector<std::string> export_as_vec(std::vector<Request>&&)
	{
		assert(false && "not implemented");
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

	auto request(std::vector<Request>&& requests)
	{
		assert(requests.begin() != requests.end() && "requests parameter must not be empty.");

		std::sort(requests.begin(), requests.end(), 
			[](const Request& r1, const Request& r2) constexpr { return r1.start < r2.start; });

		//size_t maxDB = 0;
		//for (auto& curReq : requests)
		//	if (maxDB < r.dbNum)
		//		maxDB = r./*We are better in C++ than you*/dbNum;
		//
		//for (auto [request, curDB, lastReq] = std::tuple(requests.begin(), 0, 0); curDB < maxDB; ++curDB)
		//	if (curDB == request->dbNum)
		//	{
		//		const auto sum = request->start + request->size + 1;
		//		if (++request == requests.end())
		//			request = requests.begin();
		//
		//		if (sum != request->start)
		//		{
		//			lastReq = sum;
		//			m_req->push_request(Request{ request->area, curDB, lastReq, sum - 1 });
		//		}
		//	}

		for (auto [request, lastReqDB, data_sum] = std::tuple(requests.begin(), 0, std::string()); true;)
		{
			const auto sum = request->start + request->size + 1;
			const auto prev_db = request->dbNum;
			data_sum.append(request->data);

			if (++request == requests.end())
			{
				m_req->push_request(Request{ daveDB, prev_db, lastReqDB, sum - 1, std::move(data_sum) });
				m_req->send();
				break;
			}

			if (sum != request->start || prev_db != request->dbNum)
			{
				m_req->push_request(Request{ daveDB, prev_db, lastReqDB, sum - 1, std::move(data_sum) });
				lastReqDB = request->start;
				//data_sum.clear();
			}
		}

		return m_req->export_as_vec(requests);
	}

private:
	std::unique_ptr<ISPSRequest> m_req;
};


using SPS = basic_SPS<ESPSIO>;