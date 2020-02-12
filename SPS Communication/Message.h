#pragma once
#include "Includes.h"
#include "Command.h"
#include "Parser.h"

template<typename _Type = void*, template<typename> class... Ex>
class basic_CommandList : public Ex<basic_CommandList<_Type, Ex...>>...
{
public:
	using type_t = _Type;
	static constexpr std::chrono::seconds DEFAULT_TIME = 10s;

	basic_CommandList() = default;

	const auto& timeout() const noexcept { return m_timeout; }
	void timeout(const std::chrono::seconds& t) noexcept { m_timeout = t; }

	const auto& element() const noexcept { return m_ele; }
	void element(const _Type& ele) noexcept { m_ele = ele; }

	auto& element() noexcept { return m_ele; }

private:
	std::chrono::seconds m_timeout = DEFAULT_TIME;
	_Type m_ele;
};


template<typename Impl>
class EListParser
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	static constexpr char COM_SEP = '\n';

	EListParser() = default;

	//NOTICE: Automaticaly handles requesttimeout variable.
	void parse(std::string_view message)
	{
		using type_t = typename Impl::type_t;

		Parser p;
		p.data(message);

		bool req_found = false;

		if (p.is_same("[requesttimeout]=>"))
			req_found = true,
			_extract_time_(p);
		else if constexpr (!std::is_same_v<type_t, void*>)
		{
			if (const auto str = p.get_until(COM_SEP); str.has_value())
				underlying()->element().parse(str.value());
			else
				throw Logger("state or authcode variable not found.");

			if (p.is_same("[requesttimeout]=>"))
				req_found = true,
				_extract_time_(p);

			if (!req_found)
				throw Logger("requesttimeout missing.");
		}
		else
			throw Logger("requesttimeout missing.");
	}

private:
	void _extract_time_(Parser& p)
	{
		if (const auto time_int = p.get_num<unsigned long long>(COM_SEP); time_int.has_value())
			underlying()->timeout(std::chrono::seconds(time_int.value()));
		else 
			throw Logger("Time isn't readable.");
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

		//buf += 

		for (auto& com : *underlying())
		{
			for (auto& var : com)
			{

			}
		}
	}

};


template<typename _Type>
using CommandList = basic_CommandList<_Type, EListParser>;







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