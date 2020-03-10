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
		//Init parser
		Parser parser;
		parser.data(message);

		//Check if #START is present
		g_log.initiate("#START check");
		if (!parser.is_same(HEADERS[START]))
			throw Logger("Missing #START.");
		g_log.complete();

		//Check if #DEBUG is present and pass to func
		g_log.initiate("#DEBUG check");
		if (parser.is_same(HEADERS[DEBUG]))
			if (const auto temp = parser.get_until('#'); temp.has_value())
				this->_handle_debug(temp.value());
			else
				throw Logger("Missing #DEBUG message.");
		g_log.complete();

		//Check if #DATA is present and pass to func
		g_log.initiate("#DATA extractor");
		if (!parser.is_same(HEADERS[DATA]))
			throw Logger("Missing #DATA.");
		if (const auto temp = parser.get_until('#'); temp.has_value())
			this->_handle_data(temp.value());
		else
			throw Logger("Missing #DATA message.");
		
		g_log.complete();

		//Check if #END is present
		g_log.initiate("#END check");
		if (!parser.is_same(HEADERS[END]))
			throw Logger("Missing #END.");
		g_log.complete();
	}

private:

};

template<typename Impl>
class EDebugHandler : public crtp<Impl, EDebugHandler>
{
public:
	EDebugHandler() = default;

protected:
	void _handle_debug(std::string_view debug)
	{
		g_log.write(std::string("DEBUG: ").append(debug));
	}

};


template<typename Impl>
class EDataHandler : public crtp<Impl, EDataHandler>
{
public:
	EDataHandler() = default;

	template<typename... T>
	auto& get_var(const char* first, T&&... names)
	{
		auto* level = &m_d[first];
		((level = &(*level)[names]), ...);
		return *level;
	}

	const auto& data() const noexcept { return m_d; }
	auto&& give_data() noexcept { return std::move(m_d); }

protected:
	void _handle_data(std::string_view data)
	{
		if (m_d.Parse<rapidjson::kParseNumbersAsStringsFlag>(data.data(), data.size()).HasParseError())
			throw Logger(rapidjson::GetParseError_En(m_d.GetParseError()));
	}

private:
	rapidjson::Document m_d;

};

