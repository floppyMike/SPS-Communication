#pragma once
#include "Includes.h"

struct Results
{
	std::string header, content;
};

template<typename session_t>
Results query(asio::io_context& io, std::string_view host, std::string_view path)
{
	session_t session(io);

	std::clog << "Connecting to Host...";
	tcp::resolver r(io);
	tcp::resolver::query q(std::string(host), "http");
	asio::connect(session.socket(), r.resolve(q));
	std::clog << "Done\n";

	std::clog << "Reading message...";
	auto message = session.query(host, path);
	std::clog << "Done\n";

	std::clog << "\n----------------------------------------\n\n";

	return message;
}


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

	auto query(std::string_view host, std::string_view path)
	{
		_send_request_(host, path);
		_validate_reponse_();
		_read_headers_();
		_read_content_();
		
		return std::move(m_message);
	}

	tcp::socket& socket() noexcept
	{
		return m_socket;
	}

private:
	tcp::socket m_socket;
	asio::streambuf m_buf;

	Results m_message;


	void _send_request_(std::string_view host, std::string_view path)
	{
		std::ostream(&m_buf) << "GET " << path << " HTTP/1.0\r\n"
			<< "Host: " << host << "\r\n"
			<< "Accept: */*\r\n"
			<< "Connection: close\r\n\r\n";

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

		asio::streambuf::const_buffers_type buf_type = m_buf.data();
		m_message.header = std::string(asio::buffers_begin(buf_type), asio::buffers_begin(buf_type) + n - 4);

		m_buf.consume(n);
	}

	void _read_content_()
	{
		while (true)
		{
			std::error_code err;
			const auto size = asio::read(m_socket, m_buf, err);

			if (err == asio::error::eof)
			{
				asio::streambuf::const_buffers_type buf_type = m_buf.data();
				m_message.content.append(asio::buffers_begin(buf_type), asio::buffers_begin(buf_type) + size);
				break;
			}
			else if (err)
				throw std::runtime_error(err.message());

			asio::streambuf::const_buffers_type buf_type = m_buf.data();
			m_message.content.append(asio::buffers_begin(buf_type), asio::buffers_begin(buf_type) + size);
		}
	}
};
