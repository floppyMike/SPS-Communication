#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"

class RequestProcessing
{
public:
	enum HeaderList { START, DEBUG, DATA, END };
	static constexpr std::array<std::string_view, 4> HEADERS = { "#START\n", "#DEBUG\n", "#DATA\n", "#END" };

	RequestProcessing() = default;

	void parse(std::string_view message)
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
				m_debug = temp.value();
			else
				throw Logger("Missing #DEBUG message.");
		g_log.complete();

		g_log.initiate("#DATA extractor");
		if (!parser.is_same(HEADERS[DATA]))
			throw Logger("Missing #DATA.");
		if (const auto temp = parser.get_until('#'); temp.has_value())
			m_data = temp.value();
		else
			throw Logger("Missing #DATA message.");
		g_log.complete();

		g_log.initiate("#END check");
		if (!parser.is_same(HEADERS[END]))
			throw Logger("Missing #END.");
		g_log.complete();
	}

	void print_debug()
	{
		g_log.write("DEBUG: " + m_debug);
	}

	const auto& data() const noexcept { return m_data; }

private:
	std::string m_debug;
	std::string m_data;
};
