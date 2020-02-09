#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Message.h"


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

struct ByteArray
{
	std::vector<char> array;
	int db;
};

template<typename Com_t>
ByteArray command_to_bytearray(const Com_t& com)
{
	ByteArray arr;
	arr.db = com.db();

	for (auto& i : com)
		arr.array.insert(arr.array.end(), i.begin(), i.end());

	return arr;
}

template<typename Com_t>
Com_t bytearray_to_command(const ByteArray& arr, Com_t&& command_origin)
{
	Com_t com;
	com.db(arr.db);

	auto iter = arr.array.begin();
	size_t bool_count = 0;
	for (auto iter_com = command_origin.begin(); iter_com != command_origin.end(); ++iter_com)
	{
		if (iter_com->type == Com_t::BOOL)
		{
			com.emplace_back(std::vector<char>{ 1, *iter & (1 << bool_count++) }, iter_com->type);
			//if (iter_com + 1 == command_origin.end() || (iter_com + 1)->type != Com_t::BOOL)
			//	bool_count = 0;
		}
		else
		{
			bool_count = 0;
			com.emplace_back(std::vector<char>{ iter, iter + Com_t::TYPE_SIZE[iter_com->type] / 8 }, iter_com->type);
			iter += Com_t::TYPE_SIZE[iter_com->type] / 8;
		}
	}

	return com;
}

class ISPSRequest
{
public:
	ISPSRequest(daveConnection* c)
		: m_con(c)
	{
	}

	virtual ~ISPSRequest()
	{
		for (auto& i : m_result)
			daveFreeResults(&i);
	}

	ByteArray results(int db)
	{
		ByteArray arr;
		arr.db = db;

		for (auto& i : m_result)
			for (auto* iter_res = i.results; iter_res != i.results + i.numResults; ++iter_res)
				arr.array.insert(arr.array.end(), iter_res->bytes, iter_res->bytes + iter_res->length);

		return arr;
	}

protected:
	daveConnection* m_con;
	PDU m_p;

	std::vector<daveResultSet> m_result;
};

class SPSReadRequest : ISPSRequest
{
public:
	static constexpr int PDU_READ_LIMIT = 222;

	SPSReadRequest(daveConnection* c)
		: ISPSRequest(c)
	{
		davePrepareReadRequest(c, &m_p);
	}

	void add_vars(int db, int len)
	{
		assert(len <= PDU_READ_LIMIT && "Read request to SPS to large.");

		if (m_curr_size += len >= PDU_READ_LIMIT)
			send();

		daveAddVarToReadRequest(&m_p, daveDB, db, 0, len);
	}

	void send()
	{
		m_result.resize(m_result.size() + 1);

		m_curr_size = 0;
		if (auto res = daveExecReadRequest(m_con, &m_p, &m_result.back()); res != 0)
			throw Logger(daveStrerror(res));
	}

	using ISPSRequest::results;

private:
	int m_curr_size = 0;
};

class SPSWriteRequest : ISPSRequest
{
public:
	static constexpr size_t PDU_WRITE_LIMIT = 222;

	SPSWriteRequest(daveConnection* c)
		: ISPSRequest(c)
	{
		davePrepareWriteRequest(c, &m_p);
	}

	void add_vars(const ByteArray& arr)
	{
		assert(arr.array.size() <= PDU_WRITE_LIMIT && "Read request to SPS to large.");

		if (m_curr_size += arr.array.size() >= PDU_WRITE_LIMIT)
			send();

		daveAddVarToWriteRequest(&m_p, daveDB, arr.db, 0, arr.array.size(), const_cast<char*>(arr.array.data()));
	}

	void send()
	{
		m_result.resize(m_result.size() + 1);

		m_curr_size = 0;
		if (auto res = daveExecWriteRequest(m_con, &m_p, &m_result.back()); res != 0)
			throw Logger(daveStrerror(res));
	}

	using ISPSRequest::results;

private:
	int m_curr_size = 0;
};

template<typename Impl>
class ESPSIO
{
	const Impl* underlying() const noexcept { return static_cast<Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	ESPSIO() = default;

	template<typename ReadSession>
	ByteArray in(int db, int len)
	{
		assert(len != 0);

		ReadSession read(underlying()->connection_ptr());

		for (int curr_len = std::clamp(len, 0, ReadSession::PDU_READ_LIMIT); len > 0; curr_len = std::clamp(len, 0, ReadSession::PDU_READ_LIMIT))
		{
			read.add_vars(db, curr_len);
			len -= curr_len;
		}

		read.send();
		return read.results(db);
	}

	template<typename WriteSession>
	void out(const ByteArray& bytes)
	{
		assert(bytes.array.size() != 0);

		WriteSession write(underlying()->connection_ptr());

		auto iter_beg = bytes.array.begin(), iter_end = bytes.array.begin() + bytes.array.size();
		while (std::distance(iter_beg, iter_end) <= WriteSession::PDU_WRITE_LIMIT)
		{
			iter_end = iter_beg + WriteSession::PDU_WRITE_LIMIT;
			write.add_vars({ { iter_beg, iter_end }, bytes.db });
			iter_beg = iter_end;
		}
		write.add_vars({ { iter_beg, bytes.array.end() }, bytes.db });

		write.send();
		return write.results(bytes.db);
	}

};


using SPS = basic_SPS<ESPSIO>;