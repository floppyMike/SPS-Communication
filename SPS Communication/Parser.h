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

	std::string_view data() const noexcept { return m_data; }

	void reset() noexcept
	{
		m_loc = 0u;
	}

	auto find(char delim)
	{
		return m_data.find(delim, m_loc);
	}

	std::optional<std::string_view> get_until_w_end(char delim)
	{
		if (const auto loc = find(delim); loc != std::string_view::npos)
			return get_until(loc + 1 - m_loc);

		return std::nullopt;
	}

	std::optional<std::string_view> get_until(char delim)
	{
		if (const auto loc = find(delim); loc != std::string_view::npos)
			return get_until(loc - m_loc);

		return std::nullopt;
	}
	template<size_t _num>
	std::optional<std::string_view> get_until(const std::array<char, _num>& l)
	{
		for (auto iter = m_data.begin() + m_loc; iter != m_data.end(); ++iter)
			for (const auto& i : l)
				if (*iter == i)
					return get_until(static_cast<size_t>(std::distance(m_data.begin() + m_loc, iter)));

		return std::nullopt;
	}
	std::string_view get_until(size_t count)
	{
		assert(m_loc + count <= m_data.size() && "String is too short");

		auto&& sub_data = m_data.substr(m_loc, count);
		m_loc += count;

		return sub_data;
	}

	bool skip_until(char delim)
	{
		if (const auto loc = find(delim); loc != std::string_view::npos)
		{
			m_loc = loc + 1;
			return false;
		}

		return true;
	}
	bool skip(size_t num) noexcept
	{
		if (m_loc + num > m_data.size())
			return false;

		m_loc += num;
		return true;
	}

	bool is_same(std::string_view str) noexcept
	{
		if (m_data.size() - m_loc < str.size())
			return false;

		const auto res = m_data.size() - m_loc >= str.size() && std::equal(&m_data[m_loc], &m_data[m_loc + str.size() - 1], str.data());

		if (res)
			skip(str.size());

		return res;
	}

	auto current_loc() const noexcept
	{
		return m_loc;
	}

	template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>>>
	std::optional<T> get_num(char delim)
	{
		if (const auto end = find(delim); end == std::string_view::npos)
		{
			const auto temp = str_to_num<T>(m_data.substr(m_loc, end));

			if (temp.has_value())
				m_loc = end + 1;

			return temp;
		}
	}

	std::optional<char> peek()
	{
		if (m_loc + 1 >= m_data.size())
			return std::nullopt;

		return m_data[m_loc + 1];
	}

	bool at_end() const noexcept
	{
		return m_loc == m_data.size();
	}

	void mov(int dis)
	{
		m_loc += dis;
	}

private:
	std::string_view m_data;
	size_t m_loc = 0u;
};
