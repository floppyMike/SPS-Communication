#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"
#include "utility.h"
#include "JSON.h"

class ResponseHandler
{
public:
	enum HeaderList
	{
		START,
		DEBUG,
		DATA,
		END
	};
	static constexpr std::array<std::string_view, 4> HEADERS = { "#START\n", "#DEBUG\n", "#DATA\n", "#END" };

	ResponseHandler() = default;

	static auto parse_content(std::string_view message) -> JSONRoot
	{
		g_log.write(Logger::Catagory::INFO) << "Checking message contents with the size of " << message.size();

		Parser parser;
		parser.data(message);

		g_log.write(Logger::Catagory::INFO, "Checking start");
		_check_start_(parser);

		g_log.write(Logger::Catagory::INFO, "Handling debug");
		_check_print_debug_(parser);

		g_log.write(Logger::Catagory::INFO, "Handling data");
		_check_data_(parser);
		JSONRoot json = extract_data(parser);

		g_log.write(Logger::Catagory::INFO, "Checking end");
		_check_end_(parser);

		return json;
	}

private:
	static void _check_start_(Parser &p)
	{
		if (!p.is_same(HEADERS[START]))
			throw std::runtime_error("Missing #START.");
	}

	static void _check_print_debug_(Parser &p)
	{
		if (p.is_same(HEADERS[DEBUG]))
			g_log.write(Logger::Catagory::INFO)
				<< "Debug message: " << guarded_get(p.get_until('#'), "Missing #DEBUG message.");
	}

	static void _check_data_(Parser &p)
	{
		if (!p.is_same(HEADERS[DATA]))
			throw std::runtime_error("Missing #DATA.");
	}
	static auto extract_data(Parser &p) -> JSONRoot { return JSONRoot(guarded_get(p.get_until('#'), "Missing #DATA message.")); }

	static void _check_end_(Parser &p)
	{
		if (!p.is_same(HEADERS[END]))
			throw std::runtime_error("Missing #END.");
	}
};
