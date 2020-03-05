#pragma once
#include "Includes.h"
#include "Logging.h"
#include "utility.h"


using Parameter = std::pair<std::string_view, std::string_view>;
struct Results { std::string header, content; };

template<template<typename> class... Ex>
class basic_Query : public Ex<basic_Query<Ex...>>...
{
public:
	basic_Query() = default;

	template<typename session_t, typename... _Para>
	Results query(asio::io_context& io, std::string_view host, std::string_view path, _Para&&... parameters) const
	{
		session_t session(io);

		g_log.initiate("connection to host");
		tcp::resolver r(io);
		tcp::resolver::query q(host.data(), "http");
		asio::connect(session.socket(), r.resolve(q));
		g_log.complete();

		g_log.initiate("message reader");
		const auto message = session.query(this->_build_req(host, path, this->_build_para(parameters...)));
		g_log.complete();

		return message;
	}

};

template<typename Impl>
class EGETBuilder : public crtp<Impl, EGETBuilder>
{
public:
	EGETBuilder() = default;

protected:
	std::string _build_req(std::string_view host, std::string_view path, std::string_view parameters) const
	{
		std::string chain("GET ");
		chain.append(path);

		if (!parameters.empty())
			chain.push_back('?'),
			chain.append(parameters);

		return chain.append(" HTTP/1.0\r\nHost: ").append(host).append("\r\n\r\n");
	}
};

template<typename Impl>
class EPOSTBuilder : public crtp<Impl, EPOSTBuilder>
{
public:
	EPOSTBuilder() = default;

protected:
	std::string _build_req(std::string_view host, std::string_view path, std::string_view parameters) const
	{
		return std::string("POST ").append(path).append(" HTTP/1.0\r\nHost: ").append(host)
			.append("\r\n").append(parameters).append("\r\n\r\n");
	}
};

template<typename Impl>
class EParamBuilder : public crtp<Impl, EParamBuilder>
{
public:
	EParamBuilder() = default;

protected:
	template<typename... _Para>
	std::string _build_para(_Para&&... paras) const
	{
		std::string str;

		((str += paras.first + '=' + paras.second + '&'), ...);

		if (!str.empty())
			str.pop_back();

		return str;
	}

};


class Session
{
public:
	Session(asio::io_context& io)
		: m_socket(io)
	{
	}

	~Session()
	{
		std::error_code err;
		m_socket.close(err);
		if (err)
			std::clog << "Socket didn't close correctly. Message: " << err.message() << '\n';
	}

	auto& query(std::string_view head)
	{
		_send_request_(head);
		_validate_reponse_();
		_read_headers_();
		_read_content_();

		return m_message;
	}

	tcp::socket& socket() noexcept
	{
		return m_socket;
	}

private:
	tcp::socket m_socket;
	asio::streambuf m_buf;

	Results m_message;

	void _send_request_(std::string_view head)
	{
		std::ostream(&m_buf) << head;
		asio::write(m_socket, m_buf);
	}

	void _validate_reponse_()
	{
		asio::read_until(m_socket, m_buf, "\r\n");
		std::istream response_s(&m_buf);

		std::string http_version;
		unsigned int status_code;
		response_s >> http_version >> status_code;

		std::string status_message;
		std::getline(response_s, status_message);

		if (!response_s || http_version.substr(0, 5) != "HTTP/")
			throw std::runtime_error("Invalid response");
		if (status_code != 200)
			throw std::runtime_error("Response returned with status code " + std::to_string(status_code) + ". Message: " + status_message);
	}

	void _read_headers_()
	{
		const auto n = asio::read_until(m_socket, m_buf, "\r\n\r\n");
		m_message.header = _buf_to_str_(n - 4);
		m_buf.consume(n);
	}

	void _read_content_()
	{
		while (true)
		{
			std::error_code err;
			const auto n = m_buf.size() + asio::read(m_socket, m_buf, err);

			if (err == asio::error::eof)
			{
				m_message.content.append(_buf_to_str_(n));
				break;
			}
			else if (err)
				throw std::runtime_error(err.message());

			m_message.content.append(_buf_to_str_(n));
			m_buf.consume(n);
		}
	}

	std::string _buf_to_str_(size_t len)
	{
		asio::streambuf::const_buffers_type buf_type = m_buf.data();
		return std::string(asio::buffers_begin(buf_type), asio::buffers_begin(buf_type) + len);
	}
};