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

		_check_start_(parser);
		_check_debug_(parser);
		_check_data_(parser);
		_check_end_(parser);
	}

private:

	void _check_start_(Parser& p)
	{
		g_log.initiate("#START check");
		if (!p.is_same(HEADERS[START]))
			throw Logger("Missing #START.");
		g_log.complete();
	}

	void _check_debug_(Parser& p)
	{
		g_log.initiate("#DEBUG check");
		if (p.is_same(HEADERS[DEBUG]))
			this->_handle_debug(guarded_get(p.get_until('#'), "Missing #DEBUG message."));
		g_log.complete();
	}

	void _check_data_(Parser& p)
	{
		g_log.initiate("#DATA extractor");
		if (!p.is_same(HEADERS[DATA]))
			throw Logger("Missing #DATA.");
		this->_handle_data(guarded_get(p.get_until('#'), "Missing #DATA message."));
		g_log.complete();
	}

	void _check_end_(Parser& p)
	{
		g_log.initiate("#END check");
		if (!p.is_same(HEADERS[END]))
			throw Logger("Missing #END.");
		g_log.complete();
	}
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

