#pragma once

#include "Includes.h"
#include "Logging.h"
#include "utility.h"

class Variable
{
public:
	enum Type
	{
		BOOL,
		BYTE,
		WORD,
		DWORD,
		CHAR,
		INT,
		DINT,
		REAL,
		MAX
	};
	static constexpr std::array<size_t, Type::MAX>			 TYPE_SIZE = { 1, 1, 2, 4, 1, 2, 4, 4 };
	static constexpr std::array<std::string_view, Type::MAX> TYPE_STR  = { "BOOL", "BYTE", "WORD", "DWORD",
																		   "CHAR", "INT",  "DINT", "REAL" };

public:
	Variable(std::string_view var_name, Type t)
		: m_type(t)
		, m_name(var_name)
		, m_data(TYPE_SIZE[t], 0)
	{
	}

	void fill_var(std::string_view str_val)
	{
		switch (m_type)
		{
		case Type::BOOL:
		case Type::BYTE: fill_var<uint8_t>(_get_num_<uint8_t>(str_val)); break;
		case Type::CHAR: fill_var<int8_t>(_get_num_<int8_t>(str_val)); break;
		case Type::WORD: fill_var<uint16_t>(_get_num_<uint16_t>(str_val)); break;
		case Type::INT: fill_var<int16_t>(_get_num_<int16_t>(str_val)); break;
		case Type::DINT: fill_var<int32_t>(_get_num_<int32_t>(str_val)); break;
		case Type::DWORD: fill_var<uint32_t>(_get_num_<uint32_t>(str_val)); break;
		case Type::REAL: fill_var<float>(_get_num_<float>(str_val)); break;
		default: throw std::runtime_error("Undefinied type."); break;
		}
	}

	template<typename _T, typename = typename std::enable_if_t<std::is_arithmetic_v<_T>>>
	void fill_var(_T val)
	{
		// Fill bytes to reserved vector
		assert(sizeof(_T) == m_data.size() && "Value size isn't the matching the type of variable.");
		*reinterpret_cast<_T *>(&m_data.front()) = val;

		// Reverse bytes to match SPS
		std::reverse(m_data.begin(), m_data.end());
	}

	[[nodiscard]] auto byte_size() const noexcept { return TYPE_SIZE[m_type]; }

	void fill_var(const std::vector<uint8_t> &val) { m_data = val; }
	void fill_var(std::vector<uint8_t> &&val) noexcept { m_data = std::move(val); }

	[[nodiscard]] auto data() const noexcept -> const auto & { return m_data; }
	[[nodiscard]] auto type() const noexcept -> const auto & { return m_type; }

	void			   name(std::string_view str) noexcept { m_name = str; }
	void			   name(std::string &&str) noexcept { m_name = std::move(str); }
	[[nodiscard]] auto name() const noexcept -> std::string_view { return m_name; }

	[[nodiscard]] auto type_str() const noexcept -> std::string_view { return TYPE_STR[m_type]; }

	[[nodiscard]] auto val_str() const -> std::string
	{
		// Prepare data
		auto dat = m_data;
		std::reverse(dat.begin(), dat.end());

		switch (m_type)
		{
		case Type::BOOL:
		case Type::BYTE: return std::to_string(*reinterpret_cast<const uint8_t *>(&dat.front()));
		case Type::WORD: return std::to_string(*reinterpret_cast<const uint16_t *>(&dat.front()));
		case Type::DWORD: return std::to_string(*reinterpret_cast<const uint32_t *>(&dat.front()));
		case Type::CHAR: return std::to_string(*reinterpret_cast<const int8_t *>(&dat.front()));
		case Type::INT: return std::to_string(*reinterpret_cast<const int16_t *>(&dat.front()));
		case Type::DINT: return std::to_string(*reinterpret_cast<const int32_t *>(&dat.front()));
		case Type::REAL: return std::to_string(*reinterpret_cast<const float *>(&dat.front()));
		default: return "";
		}
	}

	friend auto operator<<(std::ostream &o, const Variable &v) -> std::ostream &
	{
		o << "Name: " << v.m_name << "\t Type: " << v.type_str() << "\t Value: " << v.val_str() << "\t Bytes: ";

		for (const auto &i : v.m_data) o << std::hex << +i << ' ';

		return o;
	}

	auto operator==(const Variable &v) const noexcept -> bool
	{
		return m_type == v.m_type && m_name == v.m_name && m_data == v.m_data;
	}

private:
	Type				 m_type;
	std::string			 m_name;
	std::vector<uint8_t> m_data;

	template<typename T>
	auto _get_num_(std::string_view s) -> T
	{
		return guarded_get(str_to_num<T>(s), "Value of variable isn't convertable.");
	}
};
