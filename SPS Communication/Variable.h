#pragma once

#include "Includes.h"

class Variable
{
public:
	enum Type { BOOL, BYTE, WORD, DWORD, CHAR, INT, DINT, REAL, MAX };
	static constexpr std::array<size_t, MAX> TYPE_SIZE = { 1, 8, 16, 32, 8, 16, 32, 32 };

	Variable(Type t)
		: m_type(t)
	{
	}

	//NOTICE: When type is BOOL, previous data is not reset.
	template<typename T>
	void set(T dat)
	{
		if (m_type == BOOL)
			m_data.push_back(dat);
		else
		{
			bool_dis = 0;
			m_data.clear();

			_fill_var_<T>(dat);
		}
	}

private:
	std::vector<uint8_t> m_data;
	Type m_type;

	inline static size_t bool_dis = 0;

	void _push_bit_(uint8_t val)
	{
		if (const auto is_full = bool_dis == 8; bool_dis == 0 || is_full)
		{
			m_data.emplace_back();
			if (is_full)
				bool_dis = 0;
		}

		m_data.back() |= val << bool_dis++;
	}

	template<typename T>
	void _fill_var_(T val)
	{
		m_data.resize(sizeof(T));
		*reinterpret_cast<T*>(&m_data.front()) = val;
	}
};

