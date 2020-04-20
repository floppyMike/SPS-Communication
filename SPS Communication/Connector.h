#pragma once

#include "Includes.h"
#include "Logging.h"

class Connector
{
public:
	Connector() = default;

	Connector(asio::io_context* io)
		: m_io(io)
	{
	}

	Connector(asio::io_context* io, std::string_view host)
		: m_io(io)
		, m_host(host)
	{
	}

	void io(asio::io_context& i) noexcept { m_io = &i; }
	void host(std::string_view host) noexcept { m_host = host; }

	template<typename Builder, typename _Array>
	std::string query(std::string_view path, const _Array& para)
	{
		Session session(*m_io);

		tcp::resolver r(*m_io);
		tcp::resolver::query q(m_host.data(), "http");
		asio::connect(session.socket(), r.resolve(q));

		auto res = session.query(Builder().build_req(m_host, path, ParamBuilder().build_para(para)));

		g_log.write(Logger::Catagory::INFO) << "Queried " << m_host;
		return res.content;
	}

private:
	asio::io_context* m_io = nullptr;
	std::string_view m_host;
};


class ConnectorDEBUG
{
public:
	ConnectorDEBUG() = default;

	ConnectorDEBUG(asio::io_context* io)
		: m_io(io)
	{
	}

	ConnectorDEBUG(asio::io_context* io, std::string_view host)
		: m_io(io)
		, m_host(host)
	{
	}

	void host(std::string_view host) noexcept { m_host = host; }
	void io(asio::io_context& i) noexcept { m_io = &i; }

	template<typename Builder, typename _Array>
	std::string query(std::string_view path, const _Array& para)
	{
		if constexpr (std::is_same_v<Builder, GETBuilder>)
		{
			std::ifstream in(m_host.data(), std::ios::in | std::ios::binary);
			if (!in)
				throw std::runtime_error(std::string("File ").append(m_host) + " doesn't exist.");

			std::stringstream s;
			s << in.rdbuf();
			return s.str();
		}
		else
		{
			std::ofstream out(m_host.data(), std::ios::out | std::ios::binary);
			if (!out)
				throw std::runtime_error(std::string("File ").append(m_host) + " couldn't open.");
			out << ParamBuilder().build_para(para);
			return "#START\n#DATA\n{\"requesttimeout\": \"0\"}\n#END";
		}
	}

private:
	asio::io_context* m_io;
	std::string_view m_host;
};