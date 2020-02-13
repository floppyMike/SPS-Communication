#pragma once

#include "Includes.h"
#include "Logging.h"

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
	//template<typename T>
	//void set(T dat)
	//{
	//	if (m_type == BOOL)
	//	{
	//		if (bool_dis == 0)
	//			m_data.emplace_back();
	//		m_data.back() |= static_cast<uint8_t>(dat) << bool_dis++;
	//	}
	//	else
	//	{
	//		bool_dis = 0;
	//		m_data.clear();

	//		fill_var<T>(dat);
	//	}
	//}

	auto byte_size() const noexcept 
	{
		return TYPE_SIZE[m_type];
	}

	template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>>>
	void fill_var(T val)
	{
		m_data.resize(sizeof(T));
		*reinterpret_cast<T*>(&m_data.front()) = val;
	}

	void fill_var(const std::vector<uint8_t>& val)
	{
		m_data = val;
	}

	const auto& data() const noexcept { return m_data; }

private:
	std::vector<uint8_t> m_data;
	Type m_type;
};

