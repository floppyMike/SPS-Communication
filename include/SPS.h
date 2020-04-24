#pragma once

#include "Includes.h"
#include "Logging.h"
#include "VariableSequence.h"
#include "SPSIO.h"


class ESPSIn
{
public:
	ESPSIn() = default;

protected:
	auto _in(int db, int len, daveConnection* con)
	{
		SPSRequester<TSPSRead> read(con);

		while (len > 0)
		{
			const auto curr_len = std::clamp(len, 0, SPSRequester<TSPSRead>::PDU_LIMIT);

			read.add_vars(db, curr_len);
			len -= curr_len;
		}

		read.send();
		return read.results();
	}

};

class ESPSOut
{
public:
	ESPSOut() = default;

protected:
	auto _out(int db, const std::vector<uint8_t>& vars, daveConnection* con)
	{
		SPSRequester<TSPSWrite> write(con);

		for (auto iter = &vars.front(); iter != &vars.back();)
		{
			const auto iter_end = std::clamp(iter + SPSRequester<TSPSWrite>::PDU_LIMIT, &vars.front(), &vars.back());

			write.add_vars(db, { iter, iter_end + 1 });
			iter = iter_end;
		}

		write.send();
		return write.results();
	}

};


class SPSConnection
	: ESPSIn
	, ESPSOut
{
public:
	SPSConnection() = default;

	~SPSConnection()
	{
		if (m_connection)
			daveDisconnectPLC(m_connection);

		if (m_interface)
			daveDisconnectAdapter(m_interface);
	}

	void connect(std::string_view ip, std::string_view port, int protocol = daveProtoISOTCP)
	{
		g_log.write(Logger::Catagory::INFO) << "Connecting to SPS on port " << port;

		_open_socket_(ip, port);
		_init_interface_(protocol);
		_init_adapter_();
		_init_connection_();
	}

	auto in(int db, int len)
	{
		g_log.write(Logger::Catagory::INFO) << "Reading SPS on db " << db << " with length " << len;
		return this->_in(db, len, m_connection);
	}

	auto out(int db, const std::vector<uint8_t>& vars)
	{
		g_log.write(Logger::Catagory::INFO) << "Writing into db " << db << " bytes of size " << vars.size();
		return this->_out(db, vars, m_connection);
	}

private:
	daveInterface* m_interface = nullptr;
	daveConnection* m_connection = nullptr;
	_daveOSserialType m_serial;

	void _open_socket_(std::string_view ip, std::string_view port)
	{
		m_serial.wfd = m_serial.rfd = openSocket(guarded_get(str_to_num<int>(port), "SPS Port isn't a valid number."), const_cast<char*>(ip.data()));
		if (m_serial.rfd == 0)
			throw std::runtime_error("Couldn't open serial port " + std::string(port) + " or connect to SPS ip " + std::string(ip));
	}

	void _init_interface_(int protocol)
	{
		m_interface = daveNewInterface(m_serial, const_cast<char*>("IF1"), 0, protocol, daveSpeed187k);
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
					throw std::runtime_error("Couldn't connect to Adapter.");
			}
	}

	void _init_connection_()
	{
		m_connection = daveNewConnection(m_interface, 2, 0, 0);
		if (daveConnectPLC(m_connection) != 0)
			throw std::runtime_error("Couldn't connect to PLC.");
	}
};