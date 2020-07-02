#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"
#include "Connector.h"
#include "Interpeter.h"
#include "Stockmanager.h"
#include "utility.h"

template<typename Impl>
class Pairer : public crtp<Impl, Pairer>
{
protected:
	Pairer() = default;

	auto _pair_up(std::string_view host, std::string_view param)
	{
		g_log.write(Logger::Catagory::INFO) << "Pairing up to host: " << host;

		auto json = this->underlying()->communicate(
			[host, param, io = this->underlying()->io()] { return query(io, host, param); });

		// Get authcode from json
		const auto authcode = json.var("authcode").string();

		// Write authcode to authdata.txt
		std::ofstream fileout(PAIR_FILE_NAME.data(), std::ios::binary | std::ios::out);
		fileout << authcode;

		return authcode;
	}

	auto _pair_up(std::istream &stream)
	{
		g_log.write(Logger::Catagory::INFO) << "Pairing up using known authcode";

		std::string authcode;
		stream >> authcode;

		return authcode;
	}
};

template<typename Impl>
class PairerDebug : public crtp<Impl, PairerDebug>
{
protected:
	PairerDebug() = default;

	auto _pair_up(std::string_view file, std::string_view param)
	{
		const auto file_full = std::string(file) + "_debugpair";

		g_log.write(Logger::Catagory::WARN) << "Pairing up to host using File: " << file_full;
		g_log.write(Logger::Catagory::INFO) << "Pair request:\n" << param;

		auto json = this->underlying()->communicate([&file_full] { return query_debug_get(file_full); });

		// Get authcode from json
		const auto authcode = json.var("authcode").string();

		// Write authcode to authdata.txt
		std::ofstream fileout(PAIR_FILE_NAME.data(), std::ios::binary | std::ios::out);
		fileout << authcode;

		return authcode;
	}

	auto _pair_up(std::istream &stream)
	{
		g_log.write(Logger::Catagory::INFO) << "Pairing up using known authcode";

		std::string authcode;
		stream >> authcode;

		return authcode;
	}
};

template<typename Impl>
class Getter : public crtp<Impl, Getter>
{
protected:
	Getter() = default;

	auto _get(std::string_view host, std::string_view param)
	{
		return this->underlying()->communicate(
			[host, param, io = this->underlying()->io()] { return query(io, host, param); });
	}
};

template<typename Impl>
class GetterDebug : public crtp<Impl, GetterDebug>
{
protected:
	GetterDebug() = default;

	auto _get(std::string_view file, std::string_view param)
	{
		const auto file_full = std::string(file) + "_debugget";

		g_log.write(Logger::Catagory::WARN) << "Getting get request from file: " << file_full;
		g_log.write(Logger::Catagory::INFO) << "GET request parameters:\n" << param;

		return this->underlying()->communicate([&file_full] { return query_debug_get(file_full); });
	}
};

template<typename Impl>
class Poster : public crtp<Impl, Poster>
{
protected:
	Poster() = default;

	auto _post(std::string_view host, std::string_view param)
	{
		return this->underlying()->communicate(
			[host, param, io = this->underlying()->io()] { return query(io, host, param); });
	}
};

template<typename Impl>
class PosterDebug : public crtp<Impl, PosterDebug>
{
protected:
	PosterDebug() = default;

	auto _post(std::string_view file, std::string_view param)
	{
		const auto file_full = std::string(file) + "_debugpost";

		g_log.write(Logger::Catagory::WARN) << "Putting request to file: " << file_full;
		g_log.write(Logger::Catagory::INFO) << "POST request parameters:\n" << param;

		return this->underlying()->communicate([&file_full, param] { return query_debug_post(file_full, param); });
	}
};

template<template<typename> class... I>
class Server : public I<Server<I...>>...
{
public:
	explicit Server(asio::io_context *io)
		: m_io(io)
	{
	}

	auto pair_up(std::string_view host)
	{
		// Use auth file if available else GET from server
		std::ifstream file(PAIR_FILE_NAME.data(), std::ios::in | std::ios::binary);
		m_authcode = file
			? this->_pair_up(file)
			: this->_pair_up(host, build_get(host, "/pair.php", build_para(std::array{ Parameter{ "type", "raw" } })));
	}

	auto get(std::string_view host)
	{
		// Send GET request
		auto json =
			this->_get(host,
					   build_get(host, "/interact.php",
								 build_para(std::array{ Parameter{ "type", "raw" }, Parameter{ "authcode", m_authcode },
														Parameter{ "requesttype", "GET" } })));

		// Update stock json with Server device data
		m_doc.update_stock(json.var("data", "device"));
		return json;
	}

	template<typename _P>
	auto post(std::string_view host, std::string_view requesttype, _P &&var)
	{
		// Create json reply
		const auto str = m_doc.generate_json_reply(var).to_string();

		// POST json to server
		return this->_post(
			host,
			build_post(host, "/interact.php",
					   build_para(std::array{ Parameter{ "type", "raw" }, Parameter{ "requesttype", requesttype },
											  Parameter{ "authcode", m_authcode }, Parameter{ "data", str } })));
	}

	template<typename _Query>
	auto communicate(_Query &&q)
	{
		// Wait timeout
		g_log.write(Logger::Catagory::INFO, "Waiting through timeout...");
		std::this_thread::sleep_until(m_time_till);

		auto json = ResponseHandler::parse_content(q());

		// Update timeout
		const auto timeout = std::chrono::seconds(json.var("requesttimeout").template num<unsigned int>());
		m_time_till		   = std::chrono::steady_clock::now() + timeout;
		g_log.write(Logger::Catagory::INFO) << "Timeout duration: " << timeout.count() << 's';

		return json;
	}

	constexpr auto io() const noexcept { return m_io; }

private:
	asio::io_context *m_io;

	StockManager m_doc;

	std::string							  m_authcode;
	std::chrono::steady_clock::time_point m_time_till = std::chrono::steady_clock::now();
	JSONRoot							  m_stock;
};
