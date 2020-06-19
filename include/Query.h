#pragma once
#include "Includes.h"
#include "Logging.h"
#include "utility.h"

using Parameter = std::pair<std::string_view, std::string_view>;
struct Results
{
	std::string header, content;
};

class GETBuilder
{
public:
	GETBuilder() = default;

	std::string build_req(std::string_view host, std::string_view path, std::string_view parameters) const
	{
		std::string chain("GET ");
		chain.append(path);

		if (!parameters.empty())
			chain.push_back('?'), chain.append(parameters);

		chain.append(" HTTP/1.0\r\nHost: ").append(host).append("\r\n\r\n");

		return chain;
	}
};

class POSTBuilder
{
public:
	POSTBuilder() = default;

	std::string build_req(std::string_view host, std::string_view path, std::string_view parameters) const
	{
		return std::string("POST ")
			.append(path)
			.append(" HTTP/1.0\r\nHost: ")
			.append(host)
			.append("\r\nContent-Type: application/x-www-form-urlencoded")
			.append("\r\nContent-Length: ")
			.append(std::to_string(parameters.size()))
			.append("\r\n\r\n")
			.append(parameters)
			.append("\r\n\r\n");
	}
};

class ParamBuilder
{
public:
	ParamBuilder() = default;

	template<typename _Array>
	std::string build_para(const _Array &para) const
	{
		static_assert(!std::is_same_v<Parameter, decltype(para.front())>, "Array must be of Parameters");

		std::string str;

		for (const auto &i : para) str += std::string(i.first) + '=' + std::string(i.second) + '&';

		if (!str.empty())
			str.pop_back();

		return str;
	}
};

class Session
{
public:
	Session(asio::io_context &io)
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

	auto &query(std::string_view head)
	{
		_send_request_(head);
		_validate_reponse_();
		_read_headers_();
		_read_content_();

		return m_message;
	}

	tcp::socket &socket() noexcept { return m_socket; }

private:
	tcp::socket		m_socket;
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

		std::string	 http_version;
		unsigned int status_code;
		response_s >> http_version >> status_code;

		std::string status_message;
		std::getline(response_s, status_message);

		if (!response_s || http_version.substr(0, 5) != "HTTP/")
			throw std::runtime_error("Invalid response");
		if (status_code != 200)
			throw std::runtime_error("Response returned with status code " + std::to_string(status_code)
									 + ". Message: " + status_message);
	}

	void _read_headers_()
	{
		const auto n	 = asio::read_until(m_socket, m_buf, "\r\n\r\n");
		m_message.header = _buf_to_str_(n - 4);
		m_buf.consume(n);
	}

	void _read_content_()
	{
		while (true)
		{
			std::error_code err;
			const auto		n = m_buf.size() + asio::read(m_socket, m_buf, err);

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
