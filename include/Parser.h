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

	void reset() noexcept { m_loc = 0U; }

	auto find(char delim) { return m_data.find(delim, m_loc); }

	auto get_until(char delim) -> std::optional<std::string_view>
	{
		if (const auto loc = find(delim); loc != std::string_view::npos)
			return get_until(loc - m_loc);

		return std::nullopt;
	}
	template<size_t _num>
	auto get_until(const std::array<char, _num> &l) -> std::optional<std::string_view>
	{
		for (auto iter = m_data.begin() + m_loc; iter != m_data.end(); ++iter)
			for (const auto &i : l)
				if (*iter == i)
					return get_until(static_cast<size_t>(std::distance(m_data.begin() + m_loc, iter)));

		return std::nullopt;
	}
	auto get_until(size_t count) -> std::string_view
	{
		assert(m_loc + count <= m_data.size() && "String is too short");

		auto &&sub_data = m_data.substr(m_loc, count);
		m_loc += count;

		return sub_data;
	}

	auto skip(char delim) -> bool
	{
		if (const auto loc = find(delim); loc != std::string_view::npos)
		{
			m_loc = loc + 1;
			return false;
		}

		return true;
	}
	auto skip(size_t num) noexcept -> bool
	{
		if (m_loc + num > m_data.size())
			return false;

		m_loc += num;
		return true;
	}

	auto is_same(std::string_view str) noexcept -> bool
	{
		if (m_data.size() - m_loc < str.size())
			return false;

		const auto res = m_data.size() - m_loc >= str.size()
			&& std::equal(&m_data[m_loc], &m_data[m_loc + str.size() - 1], str.data());

		if (res)
			skip(str.size());

		return res;
	}

	[[nodiscard]] auto current_loc() const noexcept { return m_loc; }

	template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>>>
	auto get_num(char delim) -> std::optional<T>
	{
		if (const auto end = find(delim); end != std::string_view::npos)
		{
			const auto temp = str_to_num<T>(m_data.substr(m_loc, end));

			if (temp.has_value())
				m_loc = end + 1;

			return temp;
		}

		return std::nullopt;
	}
	template<typename T, typename = typename std::enable_if_t<std::is_arithmetic_v<T>>>
	auto get_num() -> std::optional<T>
	{
		if (m_loc == m_data.size())
			return std::nullopt;

		const auto temp = str_to_num<T>(m_data.substr(m_loc + 1, m_data.size() - m_loc));

		if (temp.has_value())
			m_loc = m_data.size();

		return temp;
	}

	auto peek() -> std::optional<char>
	{
		if (m_loc + 1 >= m_data.size())
			return std::nullopt;

		return m_data[m_loc + 1];
	}

	[[nodiscard]] auto at_end() const noexcept -> bool { return m_loc == m_data.size(); }

	void mov(int dis) { m_loc += dis; }

private:
	std::string_view m_data;
	size_t			 m_loc = 0U;
};
