#pragma once
#include "Includes.h"
#include "Logging.h"

template<typename T>
struct Command
{
	std::string name;
	T val;
};


template<template<typename> class... Ex>
class basic_MessageCommands : std::vector<Command<int>>, public Ex<basic_MessageCommands<Ex...>>...
{
	using vec_t = std::vector<Command<int>>;

public:
	basic_MessageCommands() = default;

	vec_t::emplace_back;

};


template<typename Impl>
class EParser
{
	Impl* pthis = static_cast<Impl*>(this);

public:
	static constexpr std::string_view START = "#START\n";
	static constexpr std::string_view DEBUG = "#DEBUG\n";
	static constexpr std::string_view DATA = "#DATA\n";
	static constexpr std::string_view END = "#END";

	static constexpr std::string_view ASSIGN = "=>";

	EParser() = default;

	void parse_message(std::string_view message)
	{
		auto ptr = message.begin();

		if (!_contains_header_(START, ptr))
			throw Logger("Message synthax incorrect. Missing #START.");
		ptr += START.size();

		if (_contains_header_(DEBUG, ptr))
			_print_debug_(message, ptr);

		if (_contains_header_(DATA, ptr))
			_extract_commands_(message, ptr);
		else
			throw Logger("Message synthax incorrect. Missing #DATA.");

		if (!_contains_header_(END, ptr))
			throw Logger("Message synthax incorrect. Missing #END.");
	}

private:
	bool _contains_header_(std::string_view header, std::string_view::const_iterator& ptr)
	{
		return std::equal(ptr, ptr + header.size() - 1, header.begin());
	}

	void _print_debug_(std::string_view message, std::string_view::const_iterator& ptr)
	{
		ptr += DEBUG.size();

		const auto dist = std::distance(message.begin(), ptr);
		const auto delta_dist = message.find('#', dist) - dist;
		g_log.write(&*ptr, delta_dist);

		ptr += delta_dist;
	}

	void _extract_commands_(std::string_view message, std::string_view::const_iterator& ptr)
	{
		ptr += DATA.size();

		for (const auto end = message.end() - END.size(); ptr < end;)
		{
			Command<int> com;
			++ptr; //Skip first bracket

			com.name = _extract_till_(message, ptr, ']');

			if (!std::equal(ptr, ptr + ASSIGN.size() - 1, ASSIGN.begin()))
				throw Logger("Message synthax incorrect. Missing =>.");
			ptr += ASSIGN.size();

			auto str = _extract_till_(message, ptr, '\n');
			std::from_chars(str.data(), str.data() + str.size(), com.val);

			pthis->emplace_back(com);
		}
	}

	std::string_view _extract_till_(std::string_view message, std::string_view::const_iterator& ptr, char delim)
	{
		const auto dist = std::distance(message.begin(), ptr);
		const auto bracket_pair = message.find(delim, dist);
		ptr += bracket_pair - dist + 1;

		return message.substr(dist, bracket_pair - dist);
	}

};


using MessageCommands = basic_MessageCommands<EParser>;