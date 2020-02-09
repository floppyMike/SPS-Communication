#pragma once
#include "Includes.h"
#include "Parser.h"


struct Variable;

template<template<typename> class... Ex>
class basic_Command : std::vector<Variable>, public Ex<basic_Command<Ex...>>...
{
	using vec_t = std::vector<Variable>;

public:
	basic_Command() = default;

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

	size_t byte_size() const noexcept
	{
		return std::accumulate(this->begin(), this->end(), 0u, [](size_t num, const Variable& i) { return num + i.data.size(); });
	}

private:
	int m_db;
};

template<typename Impl>
class EParserCom : Parser
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	enum Type { BOOL, BYTE, WORD, DWORD, CHAR, INT, DINT, REAL, MAX };
	static constexpr std::array<size_t, MAX> TYPE_SIZE = { 1, 8, 16, 32, 8, 16, 32, 32 };

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
			auto type = static_cast<Type>(this->get_num(TYPE_VAL_SEP));
			auto val = this->get_num(ROW_SEP, 16);

			if (type == BOOL)
				if (!bool_flag || bool_counter >= 7)
					underlying()->emplace_back(std::vector<char>{ 1, static_cast<char>(val) }, type),
					bool_flag = true,
					bool_counter = 0;
				else
					underlying()->back().data.back() |= val << ++bool_counter;
			else
			{
				//Set flag
				bool_flag = false;
				bool_counter = 0;

				switch (type)
				{
				case BYTE:
					underlying()->emplace_back(std::vector<char>{ 1, static_cast<char>(val) }, type);
					break;

				case WORD:
					_fill_var_<uint16_t>(val, type);
					break;

				case DWORD:
					_fill_var_<uint32_t>(val, type);
					break;

				case CHAR:
					underlying()->emplace_back(std::vector<char>{ 1, static_cast<char>(val) }, type);
					break;

				case INT:
					_fill_var_<int16_t>(val, type);
					break;

				case DINT:
					_fill_var_<int32_t>(val, type);
					break;

				case REAL:
					_fill_var_<float>(val, type);
					break;

				default:
					break;
				}
			}
		}
	}

private:
	template<typename T>
	void _fill_var_(long val, Type typ)
	{
		underlying()->emplace_back(std::vector<char>{ sizeof(T), '\0' }, typ);
		*reinterpret_cast<T*>(&underlying()->back().data.front()) = val;
	}

};


using Command = basic_Command<EParserCom>;

struct Variable
{
	Variable(std::vector<char>&& dat, Command::Type typ)
		: data(std::move(dat))
		, type(typ)
	{
	}

	std::vector<char> data;
	Command::Type type;
};

