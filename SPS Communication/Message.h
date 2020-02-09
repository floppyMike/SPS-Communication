#pragma once
#include "Includes.h"
#include "Command.h"
#include "Parser.h"


template<typename Com, template<typename> class... Ex>
class basic_MessageCommands : std::vector<Com>, public Ex<basic_MessageCommands<Com, Ex...>>...
{
	using vec_t = std::vector<Com>;

public:
	using Com_t = Com;
	static constexpr char DB_NUM = 2;

	basic_MessageCommands() = default;

	using vec_t::reserve;
	using vec_t::emplace_back;
	using vec_t::empty;
	using vec_t::begin;
	using vec_t::end;
	using vec_t::size;
	using vec_t::back;
	using vec_t::resize;
	using vec_t::pop_back;

	Com_t get()
	{
		assert(this->begin() != this->end() && "vector is empty");

		Com_t&& temp = std::move(this->back());
		this->pop_back();
		return temp;
	}

	Com_t get(int db)
	{
		auto temp_iter = std::find_if(this->begin(), this->end(),
			[&db](const Com_t& com) constexpr { return com.db() == db; });

		assert(temp_iter != this->end() && "element not found");

		Com_t&& temp = std::move(*temp_iter);
		this->erase(temp_iter);
		return temp;
	}

	const auto& timeout() const noexcept { return m_timeout; }
	void timeout(const std::chrono::seconds& t) noexcept { m_timeout = t; }

private:
	std::chrono::seconds m_timeout = 10s;
};


template<typename Impl>
class EParserMes : Parser
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	enum HeaderList { START, DEBUG, DATA, END };

	static constexpr std::array<std::string_view, 4> HEADERS = { "#START\n", "#DEBUG\n", "#DATA\n", "#END" };

	static constexpr char ROW_SEP = '|';
	static constexpr char COM_SEP = '\n';

	EParserMes() = default;

	void parse_message(std::string_view message)
	{
		this->data(message);

		g_log.initiate("#START check");
		this->is_same(HEADERS[START]);
		g_log.complete();

		g_log.initiate("#DEBUG check");
		if (this->is_same(HEADERS[DEBUG]))
			_print_debug_(message);
		g_log.complete();

		g_log.initiate("#DATA extractor");
		if (this->is_same(HEADERS[DATA]))
			_extract_commands_(message);
		g_log.complete();

		g_log.initiate("#END check");
		this->is_same(HEADERS[END]);
		g_log.complete();

		this->reset();
	}

private:
	void _print_debug_(std::string_view message)
	{
		const auto loc_size = this->find('#') - this->current_loc();
		g_log.write("DEBUG: ").write(&message[this->current_loc()], loc_size);

		this->skip(loc_size);
	}

	void _extract_commands_(std::string_view message)
	{
		//State extraction
		this->is_same("[state]=>");
		underlying()->reserve(2);
		//First DB
		underlying()->resize(underlying()->size() + 1);
		underlying()->back().parse(this->get_until(ROW_SEP));
		//Second DB
		underlying()->resize(underlying()->size() + 1);
		underlying()->back().parse(this->get_until(COM_SEP));

		//Time extraction
		this->is_same("[requesttimeout]=>");
		int time;
		const auto nl = this->find('\n');
		std::from_chars(&message[this->current_loc()], &message[nl], time);
		underlying()->timeout(std::chrono::seconds(time));
		this->skip(nl - this->current_loc() + 1);
	}
};


template<typename Impl>
class EConverterMes
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	EConverterMes() = default;
	
	std::string to_string(int db)
	{
		

		std::string buf;

		buf += 

		for (auto& com : *underlying())
		{
			for (auto& var : com)
			{

			}
		}
	}

};



using MessageCommands = basic_MessageCommands<Command, EParserMes>;







struct VarList
{
	std::string name;
	std::unique_ptr<void> var;
};


template<template<typename> class... Ex>
class basic_CommandLists : std::vector<VarList>
{
	using vec_t = std::vector<VarList>;

public:
	basic_CommandLists() = default;

	using vec_t::size;
	using vec_t::begin;
	using vec_t::end;
	using vec_t::front;
	using vec_t::back;
	using vec_t::empty;
	using vec_t::emplace_back;


};