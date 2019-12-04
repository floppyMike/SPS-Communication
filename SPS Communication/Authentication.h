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

	auto& connect(std::string_view host)
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

	asio::io_context& used_io() noexcept
	{
		return m_io;
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
		asio::streambuf buf;
		asio::read_until(pthis().socket(), buf, "\r\n");
		std::istream response_s(&buf);

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

	void _read_headers_()
	{
		asio::streambuf buf;
		asio::read_until(pthis().socket(), buf, "\r\n\r\n");
		std::istream response_s(&buf);


	}

public:
	Content request(std::string_view host, std::string_view path)
	{
		pthis().resolve_connect(host, path);

		_send_request_(host, path);
		_validate_reponse_();


	}

	auto& prep_request(std::string_view host, std::string_view path)
	{
		std::ostream r(m_buf);
		r << "GET " << path << " HTTP/1.0\r\n"
			<< "Host: " << host << "\r\n"
			<< "Accept: */*\r\n"
			<< "Connection: close\r\n\r\n";

		return *this;
	}

	auto& write()
	{
		asio::write(pthis().socket(), buf);
		return *this;
	}

	auto& read_until(std::string_view str)
	{
		asio::read_until(pthis().socket(), buf, str);
		return *this;
	}

	auto& validate()
	{

	}

private:
	asio::streambuf m_buf;
};