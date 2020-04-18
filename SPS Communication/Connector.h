#pragma once

#include "Includes.h"
#include "Logging.h"

class Connector
{
public:
	Connector() = default;

	void io(asio::io_context& i) noexcept { m_io = &i; }

	const auto& host() const noexcept { return m_host; }
	void host(std::string_view h) noexcept { m_host = h; }

	template<typename Builder, typename _Array>
	std::string query(std::string_view path, const _Array& para)
	{
		Session session(*m_io);

		tcp::resolver r(*m_io);
		tcp::resolver::query q(m_host.data(), "http");
		asio::connect(session.socket(), r.resolve(q));

		auto res = session.query(Builder().build_req(m_host, path, ParamBuilder().build_para(para)));
		g_log.write(Logger::Catagory::INFO) << "Header of server message:\n" << res.header;
		return res.content;
	}

private:
	asio::io_context* m_io = nullptr;
	std::string_view m_host;		//Must be from main char**
};


class ConnectorDEBUG
{
public:
	ConnectorDEBUG() = default;

	void io(asio::io_context& i) noexcept { m_io = &i; }

	const auto& host() const noexcept { return m_host; }
	void host(std::string_view h) noexcept { m_host = h; }

	template<typename... Args>
	std::string query(Args&&...)
	{
		std::ifstream in(m_host.data(), std::ios::in | std::ios::binary);

		if (!in)
			throw std::runtime_error(std::string("File ").append(m_host) + " doesn't exist.");

		return (std::stringstream() << in.rdbuf()).str();
	}

private:
	asio::io_context* m_io;
	std::string_view m_host;		//Must be from main char**
};