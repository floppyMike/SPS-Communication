#pragma once
#include "Includes.h"
#include "Logging.h"
#include "utility.h"

class Command
{
public:
	enum DataList
	{
		AREA, DB, START, SIZE, DATA_END
	};

	static constexpr std::array<std::string_view, 4> DATA_STR = { "Area => ", "DB => ", "Start => ", "Size => " };
	static constexpr std::string_view VAL_STR = "Value => ";

	Command() = default;

	Command(std::string_view str)
	{
		put(str);
	}

	void put(std::string_view str)
	{
		auto ptr = str.begin();

		for (size_t i = 0; i < DATA_END; ++i)
		{
			if (!std::equal(ptr, ptr + DATA_STR[i].size() - 1, DATA_STR[i]))
				throw Logger(std::string("Message is missing:") + DATA_STR[i].data());
			ptr += DATA_STR[i].size();

			auto newline = str.find('\n', std::distance(str.begin(), ptr));
			std::from_chars(&*ptr, &*(str.begin() + newline), m_data[i]);
			ptr = str.begin() + newline + 1;
		}
	}

	int get(DataList dat) const noexcept
	{
		assert(dat != DATA_END && "DATA_END isn't a valid value to lookup.");
		return m_data[dat];
	}

	const std::optional<int>& val() const noexcept { return m_val; }

private:
	std::array<int, 4> m_data;
	std::optional<int> m_val = std::nullopt;
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
		auto ptr = message.begin();

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
	bool _contains_header_(std::string_view header, std::string_view::const_iterator& ptr, std::string_view::const_iterator end_ptr)
	{
		auto temp = safe_equal(ptr, end_ptr, header.size(), header);

		if (!temp)
			throw Logger("Message synthax incorrect. Missing " + std::string(header));

		ptr += header.size();
		return temp;
	}

	void _print_debug_(std::string_view message, std::string_view::const_iterator& ptr)
	{
		const auto ptr_end = message.find('#', std::distance(message.begin(), ptr)) + message.begin();
		g_log.write("DEBUG: ").write(ptr, ptr_end);

		ptr = ptr_end + 1;
	}

	void _extract_commands_(std::string_view message, std::string_view::const_iterator& ptr)
	{
		size_t content_dis = std::distance(message.begin(), ptr);
		for (size_t head_count = 0; head_count < HEADERS.size(); ++head_count)
		{
			auto temp = message.find('\n', content_dis);
			if (temp == std::string_view::npos)
				throw Logger("Message synthax incorrect. #DATA too short.");

			content_dis = temp + 1;
		}

		pthis->emplace_back(message.substr(std::distance(message.begin(), ptr), content_dis - std::distance(message.begin(), ptr)));
	}
};


using MessageCommands = basic_MessageCommands<EParser>;