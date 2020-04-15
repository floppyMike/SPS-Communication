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
		//Send query multiple times
		for (char test_case = 1; test_case <= 5; ++test_case)
			try
			{
				Session session(*m_io);

				tcp::resolver r(*m_io);
				tcp::resolver::query q(m_host.data(), "http");
				asio::connect(session.socket(), r.resolve(q));

				auto res = session.query(Builder().build_req(m_host, path, ParamBuilder().build_para(para)));
				g_log.write(Logger::Catagory::INFO) << "Header of server message:\n" << res.header;
				return res.content;
			}
			catch (const std::exception& e)
			{
				g_log.write(Logger::Catagory::ERR, e.what());
				g_log.write(Logger::Catagory::INFO) << "Case: " << +test_case << " of 5";
			}

		//Throw error at fail
		throw std::exception("Server query failed.");
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

protected:
	template<typename Builder, typename... Args>
	std::string _query(Args&&... para)
	{
		return _debug_filereader_("data.txt");
	}

private:
	asio::io_context* m_io;
	std::string_view m_host;		//Must be from main char**


	std::string _debug_filereader_(std::string_view name)
	{
		std::ifstream in(name.data(), std::ios::in | std::ios::binary);
		return (std::stringstream() << in.rdbuf()).str();
	}
};