#pragma once
#include "Includes.h"

struct Content
{
	std::string header, content;
};

template<template<typename> class... Ex>
class AuthCode : public Ex<AuthCode<Ex...>>...
{
public:
	template<template<typename> class... T>
	AuthCode(T<AuthCode>&&... arg)
		: T<AuthCode>(std::forward<T<AuthCode>>(arg))...
	{
	}

	void code(const std::string_view& s)
	{
		m_code = s;
	}

	std::string_view code()
	{
		return m_code;
	}

private:
	std::string m_code;
};



template<template<typename> class... Ex>
class basic_Host : public Ex<basic_Host<Ex...>>...
{
public:
	basic_Host(asio::io_context& io)
		: m_io(io)
	{
	}

	auto& resolve_connect(std::string_view host)
	{
		m_socket = std::make_unique<tcp::socket>(m_io);

		tcp::resolver res(m_io);
		asio::connect(*m_socket, res.resolve(host, "http"));

		return *this;
	}

	std::string_view message() const noexcept
	{
		return m_message;
	}

	auto& message(std::string&& m) noexcept
	{
		m_message = std::move(m);
		return *this;
	}

	tcp::socket& socket() noexcept
	{
		return *m_socket;
	}

private:
	asio::io_context& m_io;
	std::string m_message;

	std::unique_ptr<tcp::socket> m_socket;
};


template<typename Impl>
class ERequester
{
	inline auto& pthis() { return *static_cast<Impl*>(this); }

	void _send_request_(std::string_view host, std::string_view path)
	{
		asio::streambuf buf;
		std::ostream r(buf);
		r << "GET " << path << " HTTP/1.0\r\n"
			<< "Host: " << host << "\r\n"
			<< "Accept: */*\r\n"
			<< "Connection: close\r\n\r\n";

		asio::write(pthis().socket(), buf);
	}

	void _validate_reponse_()
	{
		std::istream response_s(&m_buffer);
		std::string http_version;
		unsigned int status_code;
		response_s >> http_version >> status_code;

		std::string status_message;
		std::getline(response_s, status_message);

		if (!response_s || http_version.substr(0, 5) != "HTTP/")
			throw std::runtime_error("Invalid response\n");
		if (status_code != 200)
			throw std::runtime_error("Response returned with status code " + std::to_string(status_code) + "\nMessage: " + status_message);


	}

	void _read_(tcp::socket& socket)
	{
		asio::read_until(socket, m_buf);
	}

public:
	ERequester(asio::io_context& io)
		: m_io(io)
	{
	}

	Content request(std::string_view host, std::string_view path)
	{
		pthis().resolve_connect(host, path);

		_send_request_(host, path);
		

	}

private:
	asio::streambuf m_buf;
};