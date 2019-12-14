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

	void parse_message(std::string&& messagea)
	{
		auto message = std::string_view(messagea);
		auto ptr = message.begin();

		auto ptr_dist = [&message, &ptr] { return std::distance(message.begin(), ptr); };

		if (!std::equal(ptr, ptr + START.size() - 1, START.begin()))
			throw Logger("Message synthax incorrect. Missing #START.");

		ptr += START.size();

		if (std::equal(ptr, ptr + DEBUG.size() - 1, DEBUG.begin()))
		{
			ptr += DEBUG.size();

			const auto dist = ptr_dist();
			const auto delta_dist = message.find('#', dist) - dist;

			auto str = std::string(&*ptr, delta_dist);
			g_log.write(str);

			ptr += delta_dist;
		}

		if (std::equal(ptr, ptr + DATA.size() - 1, DATA.begin()))
		{
			ptr += DATA.size();

			for (const auto end = message.end() - END.size(); ptr < end;)
			{
				Command<int> com;

				++ptr;

				{
					const auto dist = ptr_dist();
					const auto bracket_pair = message.find(']', dist);
					com.name = message.substr(dist, bracket_pair - dist);
					ptr += bracket_pair - dist + 1;
				}

				if (!std::equal(ptr, ptr + ASSIGN.size() - 1, ASSIGN.begin()))
					throw Logger("Message synthax incorrect. Missing =>.");
				ptr += ASSIGN.size();

				{
					const auto dist = ptr_dist();
					const auto bracket_pair = message.find('\n', dist);
					com.val = std::stoi(std::string(message.substr(dist, bracket_pair - dist)));
					ptr += bracket_pair - dist;
				}

				++ptr;

				pthis->emplace_back(com);
			}

			if (!std::equal(ptr, ptr + END.size() - 1, END.begin()))
				throw Logger("Message synthax incorrect. Missing #END.");
		}
		else
			throw Logger("Message synthax incorrect. Missing #DATA.");
	}

private:

};


using MessageCommands = basic_MessageCommands<EParser>;