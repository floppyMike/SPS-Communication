#pragma once

#include "Includes.h"
#include "Logging.h"

class Variable
{
public:
	enum Type { BOOL, BYTE, WORD, DWORD, CHAR, INT, DINT, REAL, MAX };
	static constexpr std::array<size_t, MAX> TYPE_SIZE = { 1, 1, 2, 4, 1, 2, 4, 4 };
	static constexpr std::array<std::string_view, MAX> TYPE_STR = { "BOOL", "BYTE", "WORD", "DWORD", "CHAR", "INT", "DINT", "REAL" };

	Variable(Type t)
		: m_type(t)
	{
	}

	auto byte_size() const noexcept 
	{
		return TYPE_SIZE[m_type];
	}

	template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>>>
	void fill_var(T val)
	{
		m_data.resize(sizeof(T));
		*reinterpret_cast<T*>(&m_data.front()) = val;
		std::reverse(m_data.begin(), m_data.end());
	}

	void fill_var(const std::vector<uint8_t>& val)
	{
		m_data = val;
	}

	const auto& data() const noexcept { return m_data; }
	const auto& type() const noexcept { return m_type; }

private:
	std::vector<uint8_t> m_data;
	Type m_type;
};

