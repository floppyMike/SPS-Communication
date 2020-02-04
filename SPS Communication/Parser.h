#pragma once
#include "Includes.h"
#include "Logging.h"
#include "utility.h"

class Parser
{
public:
	Parser() = default;

	void data(std::string_view dat)
	{
		reset();
		m_data = dat;
	}

	void reset() noexcept
	{
		m_loc = 0u;
	}

	auto find(char delim)
	{
		return m_data.find(delim, m_loc);
	}

	auto get_until(char delim)
	{
		const auto delim_loc = find(delim);
		auto&& sub_data = m_data.substr(m_loc, delim_loc - m_loc);
		m_loc = delim_loc + 1;

		return sub_data;
	}

	void skip_until(char delim)
	{
		m_loc = find(delim) + 1;
	}
	void skip(size_t num)
	{
		m_loc += num;
	}

	bool is_same(std::string_view str)
	{
		assert(m_data.size() - m_loc >= str.size() && "String is too big.");

		const auto temp = m_data.size() - m_loc >= str.size() && std::equal(&m_data[m_loc], &m_data[m_loc + str.size() - 1], str.data());

		if (!temp)
			throw Logger("Message synthax incorrect. Missing " + std::string(str));

		m_loc += str.size();
		return temp;
	}

	auto current_loc() const noexcept
	{
		return m_loc;
	}

	auto get_num(char delim, int base = 10)
	{
		const auto end = find(delim);
		long temp;
		if (std::from_chars(&m_data[m_loc], &m_data[end], temp, base).ec == std::errc::invalid_argument)
			throw Logger("DB isn't a number or isn't detected.");

		m_loc = end + 1;
		return temp;
	}

	char peek()
	{
		assert(m_loc + 1 < m_data.size() && "Nothing left to output at parser.");
		return m_data[m_loc + 1];
	}

private:
	std::string_view m_data;
	size_t m_loc = 0u;
};
