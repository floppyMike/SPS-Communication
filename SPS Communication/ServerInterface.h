#pragma once

#include "Includes.h"

#include "Response.h"
#include "Query.h"

class ServerInterface
{
public:
	ServerInterface(asio::io_context& io, std::string_view host)
		: m_io(io)
		, m_host(host)
	{
	}

	void pair_up()
	{
		//Validate and parse response
		basic_ResponseHandler<EDebugHandler, EDataHandler> r;
		r.go_through_content(_query_<EGETBuilder>([this](auto& q)
			{ q.host(m_host).path("/pair.php").emplace_parameter("type", "raw"); }));

		//Convert time to seconds
		if (const auto num = str_to_num<unsigned int>(r.get_var("requesttimeout").GetString()); num.has_value())
			m_curr_timeout = std::chrono::seconds(num.value());
		else
			throw Logger("requesttimeout string unconvertable.");

		//Get authcode
		m_authcode = r.get_var("authcode").GetString();
	}

	auto get_request()
	{
		//Validate and parse response
		basic_ResponseHandler<EDebugHandler, EDataHandler> r;
		r.go_through_content(_query_<EGETBuilder>([this](auto& q)
			{ q.host(m_host).path("/interact.php").emplace_parameter("type", "raw").emplace_parameter("authcode", m_authcode); }));

		return std::pair(r.get_var("data").MemberBegin(), r.get_var("data").MemberEnd());
	}

	auto& host(std::string_view h) noexcept { m_host = h; return *this; }
	const auto& timeout_dur() const noexcept { return m_curr_timeout; }

private:
	asio::io_context& m_io;

	std::string_view m_host;		//Must be from main char**
	std::string m_authcode;
	std::chrono::seconds m_curr_timeout;

	template<template<typename> class Builder, typename _Prep>
	std::string _query_(_Prep f)
	{
		//Construct query
		basic_Query<Builder, EParamBuilder> q;
		f(q);

		//Send query multiple times
		for (char test_case = 1; test_case <= 3; ++test_case)
		{
			try
			{
				return q.query<Session>(m_io).content;
			}
			catch (const std::exception & e)
			{
				g_log.write("Failed getting authentication code. Error: " + std::string(e.what()) + '\n');
				g_log.write(std::string("Trying again... ") + static_cast<char>(test_case + '0' + 1) + " of 3.\n");
			}
		}

		//Throw error at fail
		throw Logger("");
	}
};