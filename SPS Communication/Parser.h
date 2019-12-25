#pragma once
#include "Includes.h"
#include "Logging.h"

struct Command
{
	std::string name;
	int val;
};


template<template<typename> class... Ex>
class basic_MessageCommands : std::vector<Command>, public Ex<basic_MessageCommands<Ex...>>...
{
	using vec_t = std::vector<Command>;

public:
	basic_MessageCommands() = default;

	vec_t::emplace_back;
	vec_t::empty;

	Command get()
	{
		assert(this->begin() != this->end() && "vector is empty");

		Command&& temp = std::move(this->back());
		this->pop_back();
		return temp;
	}

	Command get(std::string_view name)
	{
		auto temp_iter = std::find_if(this->begin(), this->end(), 
			[&name](const Command& com) constexpr { return com.name == name; });

		assert(temp_iter != this->end() && "element not found");

		Command&& temp = std::move(*temp_iter);
		this->erase(temp_iter);
		return temp;
	}
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

		g_log.initiate("#START check");
		if (!_contains_header_(START, ptr))
			throw Logger("Message synthax incorrect. Missing #START.");
		ptr += START.size();
		g_log.complete();

		g_log.initiate("#DEBUG check");
		if (_contains_header_(DEBUG, ptr))
			_print_debug_(message, ptr);
		g_log.complete();

		g_log.initiate("#DATA extractor");
		if (_contains_header_(DATA, ptr))
			_extract_commands_(message, ptr);
		else
			throw Logger("Message synthax incorrect. Missing #DATA.");
		g_log.complete();

		g_log.initiate("#END check");
		if (!_contains_header_(END, ptr))
			throw Logger("Message synthax incorrect. Missing #END.");
		g_log.complete();
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
		g_log.write("DEBUG: ").write(&*ptr, delta_dist);

		ptr += delta_dist;
	}

	void _extract_commands_(std::string_view message, std::string_view::const_iterator& ptr)
	{
		ptr += DATA.size();

		for (const auto end = message.end() - END.size(); ptr < end;)
		{
			Command com;
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