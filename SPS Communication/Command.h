#pragma once
#include "Includes.h"
#include "Parser.h"

using Variable = std::vector<char>;

template<template<typename> class... Ex>
class basic_Command : std::vector<Variable>, public Ex<basic_Command<Ex...>>...
{
	using vec_t = std::vector<Variable>;

public:
	basic_Command() = default;

	//basic_Command(std::string_view str)
	//{
	//	put(str);
	//}

	//void put(std::string_view str)
	//{
	//	auto ptr_loc = 0u;

	//	{ //Get area and db
	//		const auto nl = str.find('\n', ptr_loc);
	//		if (nl == std::string_view::npos)
	//			throw Logger("Data synthax wrong.");

	//		std::tie(m_area, m_db) = _fill_out_(str.substr(ptr_loc, nl));
	//		ptr_loc = nl + 1;
	//	}

	//	do
	//	{
	//		this->push_back(Variable());

	//		const auto hash = str.find('#', ptr_loc);
	//		if (hash == std::string_view::npos)
	//			throw Logger("Data synthax wrong.");

	//		std::tie(this->back().start, this->back().size) = _fill_out_(str.substr(ptr_loc, hash));
	//		ptr_loc = hash + 1;

	//		if (str[ptr_loc] == VAL_SEP)
	//		{
	//			const auto space = str.find(VAR_SEP, ptr_loc);
	//			this->back().val = std::string(&str[ptr_loc], &str[space]);
	//			ptr_loc = space + 1;
	//		}
	//		else if (str[ptr_loc] == VAR_SEP)
	//			this->back().val = std::nullopt;
	//		else
	//			throw Logger("Data synthax wrong.");
	//	} while (str[ptr_loc - 1] != '\n');
	//}

	int db() const noexcept { return m_db; }
	void db(int db) noexcept { m_db = db; }

	using vec_t::operator[];
	using vec_t::size;
	using vec_t::empty;
	using vec_t::begin;
	using vec_t::end;
	using vec_t::emplace_back;
	using vec_t::back;
	using vec_t::resize;

private:
	int m_db;

	//std::pair<int, int> _fill_out_(std::string_view str)
	//{
	//	std::pair<int, int> temp;
	//	auto* addr = &temp.first;

	//	for (size_t pos = 0; pos < str.size();)
	//	{
	//		const auto sep = str.find(LOC_SEP, pos);
	//		std::from_chars(&str[pos], &str[sep], *addr);
	//		pos = sep + 1;
	//		++addr;
	//	}

	//	return temp;
	//}
};

template<typename Impl>
class ESorter
{
	Impl* const pthis = static_cast<Impl*>(this);

public:
	void addr_sort() noexcept
	{
		std::sort(pthis->begin(), pthis->end(),
			[](const Variable& v1, const Variable& v2) { return v1.start < v2.start; });
	}
};

template<typename Impl>
class EParserCom : Parser
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	enum Type { BOOL, BYTE, WORD, DWORD, CHAR, INT, DINT, REAL };

	static constexpr char DB_SEP = ';';
	static constexpr char TYPE_VAL_SEP = ';';
	static constexpr char ROW_SEP = ';';

	EParserCom() = default;

	void parse(std::string_view str)
	{
		this->data(str);

		//Parse DB
		underlying()->db(this->get_num(DB_SEP));

		//Parse Variables
		bool bool_flag = false;
		uint8_t bool_counter = 0;
		while (this->current_loc() < str.size())
		{
			//Extract type and value
			auto type = this->get_num(TYPE_VAL_SEP);
			auto val = this->get_num(ROW_SEP, 16);

			if (type == BOOL)
				if (!bool_flag || bool_counter >= 7)
					underlying()->emplace_back(1, static_cast<char>(val)),
					bool_flag = true,
					bool_counter = 0;
				else
					underlying()->back().back() |= val << ++bool_counter;
			else
			{
				//Set flag
				bool_flag = false;
				bool_counter = 0;

				switch (type)
				{
				case BYTE:
					underlying()->emplace_back(1, static_cast<uint8_t>(val));
					break;

				case WORD:
					_fill_var_<uint16_t>(val);
					break;

				case DWORD:
					_fill_var_<uint32_t>(val);
					break;

				case CHAR:
					underlying()->emplace_back(1, static_cast<char>(val));
					break;

				case INT:
					_fill_var_<int16_t>(val);
					break;

				case DINT:
					_fill_var_<int32_t>(val);
					break;

				case REAL:
					_fill_var_<float>(val);
					break;

				default:
					break;
				}
			}
		}
	}

private:
	template<typename T>
	void _fill_var_(long val)
	{
		underlying()->emplace_back(sizeof(T), '\0');
		*reinterpret_cast<T*>(&underlying()->back().front()) = val;
	}

};



using Command = basic_Command<ESorter, EParserCom>;
