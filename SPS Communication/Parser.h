#pragma once
#include "Includes.h"
#include "Logging.h"
#include "utility.h"

class Command
{
public:
	struct Variable
	{
		int start, size;
		std::optional<int> val;
	};

	static constexpr size_t COMMANDS = 3;
	static constexpr char COM_SEP = '\n';
	static constexpr char VAL_SEP = '#';
	static constexpr char LOC_SEP = ':';
	static constexpr char VAR_SEP = ' ';

	Command() = default;

	Command(std::string_view str)
	{
		put(str);
	}

	void put(std::string_view str)
	{
		auto ptr_loc = 0u;

		{ //Get area and db
			const auto nl = str.find('\n', ptr_loc);
			if (nl == std::string_view::npos)
				throw Logger("Data synthax wrong.");

			std::tie(m_area, m_db) = _fill_out_(str.substr(ptr_loc, nl));
			ptr_loc = nl + 1;
		}

		do
		{
			m_vars.push_back(Variable());

			const auto hash = str.find('#', ptr_loc);
			if (hash == std::string_view::npos)
				throw Logger("Data synthax wrong.");

			std::tie(m_vars.back().start, m_vars.back().size) = _fill_out_(str.substr(ptr_loc, hash));
			ptr_loc = hash + 1;

			if (str[ptr_loc] == VAL_SEP)
			{
				const auto space = str.find(VAR_SEP, ptr_loc);
				std::from_chars(&str[ptr_loc], &str[space], m_vars.back().val.value());
				ptr_loc = space + 1;
			}
			else if (str[ptr_loc] == VAR_SEP)
				m_vars.back().val = std::nullopt;
			else
				throw Logger("Data synthax wrong.");
		} while (str[ptr_loc - 1] != '\n');
	}

	const auto& get(size_t set) const noexcept
	{
		return m_vars[set];
	}

	int area() const noexcept { return m_area; }
	int db() const noexcept { return m_db; }

private:
	int m_area, m_db;
	std::vector<Variable> m_vars;


	std::pair<int, int> _fill_out_(std::string_view str)
	{
		std::pair<int, int> temp;
		auto* addr = &temp.first;

		for (size_t pos = 0; pos < str.size();)
		{
			const auto sep = str.find(LOC_SEP, pos);
			std::from_chars(&str[pos], &str[sep], *addr);
			pos = sep + 1;
			++addr;
		}

		return temp;
	}
};


template<template<typename> class... Ex>
class basic_MessageCommands : std::vector<Command>, public Ex<basic_MessageCommands<Ex...>>...
{
	using vec_t = std::vector<Command>;

public:
	basic_MessageCommands() = default;

	vec_t::emplace_back;
	vec_t::empty;
	vec_t::begin;
	vec_t::end;

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

	const auto& timeout() const noexcept { return m_timeout; }
	void timeout(const std::chrono::seconds& t) noexcept { m_timeout = t; }

private:
	std::chrono::seconds m_timeout = 10s;
};


template<typename Impl>
class EParser
{
	Impl* pthis = static_cast<Impl*>(this);

public:
	enum HeaderList
	{
		START, DEBUG, DATA, END
	};

	static constexpr std::array<std::string_view, 4> HEADERS = { "#START\n", "#DEBUG\n", "#DATA\n", "#END" };

	EParser() = default;

	void parse_message(std::string_view message)
	{
		const auto end_ptr = message.end();
		auto ptr_loc = 0u;

		g_log.initiate("#START check");
		_contains_header_(HEADERS[START], ptr, end_ptr);
		g_log.complete();

		g_log.initiate("#DEBUG check");
		if (_contains_header_(HEADERS[DEBUG], ptr, end_ptr))
			_print_debug_(message, ptr);
		g_log.complete();

		g_log.initiate("#DATA extractor");
		if (_contains_header_(HEADERS[DATA], ptr, end_ptr))
			_extract_commands_(message, ptr);
		g_log.complete();

		g_log.initiate("#END check");
		_contains_header_(END, ptr, end_ptr);
		g_log.complete();
	}

private:
	std::chrono::seconds m_timeout = 10s;


	bool _contains_header_(std::string_view header, size_t& loc, std::string_view message)
	{
		auto temp = message.size() - loc > header.size() && std::equal(&header[loc], &header[loc + header.size()], header);

		if (!temp)
			throw Logger("Message synthax incorrect. Missing " + std::string(header));

		loc += header.size();
		return temp;
	}

	void _print_debug_(std::string_view message, size_t& loc)
	{
		const auto loc_size = message.find('#', loc) - loc;
		g_log.write("DEBUG: ").write(&message[loc], loc_size);

		ptr += loc_size;
	}

	void _extract_commands_(std::string_view message, size_t& loc)
	{
		{ //Extract time
			const auto nl = message.find('\n', loc);
			const auto ptr_end = loc + nl;
			int time;
			std::from_chars(&message[loc], &message[ptr_end], time);
			pthis->timeout(std::chrono::seconds(time));
			loc = ptr_end + 1;
		}

		while (ptr_loc < message.size())
		{
			const auto ptr_beg = loc;
			for (size_t head_count = 0; head_count < Command::COMMANDS; ++head_count)
			{
				auto temp = message.find('\n', loc);
				if (temp == std::string_view::npos)
					throw Logger("Message synthax incorrect. #DATA too short.");

				loc = temp + 1;
			}

			pthis->emplace_back(message.substr(ptr_beg, loc - ptr_beg));
		}
	}
};


using MessageCommands = basic_MessageCommands<EParser>;