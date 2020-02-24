#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"
#include "utility.h"

template<template<typename> class... Ex>
class basic_ResponseHandler : public Ex<basic_ResponseHandler<Ex...>>...
{
public:
	enum HeaderList { START, DEBUG, DATA, END };
	static constexpr std::array<std::string_view, 4> HEADERS = { "#START\n", "#DEBUG\n", "#DATA\n", "#END" };

	basic_ResponseHandler() = default;

	void go_through_content(std::string_view message)
	{
		Parser parser;
		parser.data(message);

		g_log.initiate("#START check");
		if (!parser.is_same(HEADERS[START]))
			throw Logger("Missing #START.");
		g_log.complete();

		g_log.initiate("#DEBUG check");
		if (parser.is_same(HEADERS[DEBUG]))
			if (const auto temp = parser.get_until('#'); temp.has_value())
				this->_handle_debug(temp.value());
			else
				throw Logger("Missing #DEBUG message.");
		g_log.complete();

		g_log.initiate("#DATA extractor");
		if (!parser.is_same(HEADERS[DATA]))
			throw Logger("Missing #DATA.");
		if (const auto temp = parser.get_until('#'); temp.has_value())
			this->_handle_data(temp.value());
		else
			throw Logger("Missing #DATA message.");
		
		g_log.complete();

		g_log.initiate("#END check");
		if (!parser.is_same(HEADERS[END]))
			throw Logger("Missing #END.");
		g_log.complete();
	}

private:

};

template<typename Impl>
class EResDebug : public crtp<Impl, EResDebug>
{
public:
	EResDebug() = default;

protected:
	void _handle_debug(std::string_view debug)
	{
		g_log.write(std::string("DEBUG: ").append(debug));
	}

};


template<typename Impl>
class EResData : public crtp<Impl, EResData>
{
public:
	EResData() = default;

	auto& get_var(const char* var_name)
	{
		return m_d[var_name];
	}

protected:
	void _handle_data(std::string_view data)
	{
		if (m_d.Parse<rapidjson::kParseNumbersAsStringsFlag>(data.data(), data.size()).HasParseError())
			throw Logger(rapidjson::GetParseError_En(m_d.GetParseError()));
	}

private:
	rapidjson::Document m_d;

};

