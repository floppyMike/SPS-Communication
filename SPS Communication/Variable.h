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

	void fill_var(std::string_view str_val)
	{
		switch (m_type)
		{
		case Variable::BOOL:
		case Variable::CHAR:
		case Variable::BYTE:
			fill_var<int8_t>(_get_num_<int8_t>(str_val));
			break;

		case Variable::INT:
		case Variable::WORD:
			fill_var<int16_t>(_get_num_<int16_t>(str_val));
			break;

		case Variable::DINT:
		case Variable::DWORD:
			fill_var<int32_t>(_get_num_<int32_t>(str_val));
			break;

		case Variable::REAL:
			fill_var<float>(_get_num_<float>(str_val));
			break;

		default:
			throw Logger("Undefinied type.");
			break;
		}
	}

	template<typename _T, typename = typename std::enable_if_t<std::is_arithmetic_v<_T>>>
	void fill_var(_T val)
	{
		m_data.resize(sizeof(_T));
		*reinterpret_cast<_T*>(&m_data.front()) = val;
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

	template<typename T>
	auto _get_num_(std::string_view s)
	{
		if (const auto val = str_to_num<T>(s); val.has_value())
			return val.value();
		else
			throw Logger("Value of variable isn't convertable.");
	}
};

