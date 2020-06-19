#pragma once

#include "Includes.h"

template<typename T, template<typename> class crtpType>
struct crtp
{
	T *		 underlying() noexcept { return static_cast<T *>(this); }
	const T *underlying() const noexcept { return static_cast<const T *>(this); }

private:
	crtp() = default;
	friend crtpType<T>;
};

template<typename T>
std::optional<T> str_to_num(std::string_view str) noexcept
{
	if (T temp; str.empty() || [&temp, &str]() -> bool {
			if constexpr (std::is_floating_point_v<T>) // g++ and clang++ don't support floating point from_chars
				if constexpr (std::is_same_v<T, float>)
					try
					{
						temp = std::stof(std::string(str));
						return false;
					}
					catch (const std::exception &)
					{
						return true;
					}
				else
				{
					assert(false && "Only floating point number float is supported.");
					return true;
				}
			else
				return std::from_chars(str.data(), str.data() + str.size(), temp).ec != std::errc();
		}())
		return std::nullopt;
	else
		return temp;
}

template<typename _T>
auto guarded_get(std::optional<_T> &&opt, std::string_view message_on_error)
{
	if (opt.has_value())
		return opt.value();
	else
		throw std::runtime_error(message_on_error.data());
}

const char *guarded_get_string(const rj::Value &val)
{
	if (val.IsString())
		return val.GetString();
	else
		throw std::runtime_error("String expected at data variable.");
}

const auto &guarded_get_section(const rj::Value &val, std::string_view sec)
{
	if (auto iter = val.FindMember(sec.data()); iter != val.MemberEnd())
		return iter->value;
	else
		throw std::runtime_error("Object \"" + std::string(sec) + "\" not found in the json data.");
}

auto &guarded_get_section(rj::Value &val, std::string_view sec)
{
	if (auto iter = val.FindMember(sec.data()); iter != val.MemberEnd())
		return iter->value;
	else
		throw std::runtime_error("Object \"" + std::string(sec) + "\" not found in the json data.");
}

// template<typename _Typ, typename _Parent>
// class Getter_Setter
//{
//	using mem_func_set_t = void (_Parent::*)(const _Typ&);
//	using mem_func_get_t = const _Typ& (_Parent::*)();
//
// public:
//	Getter_Setter() = delete;
//
//	Getter_Setter(const Getter_Setter&) = delete;
//	Getter_Setter(Getter_Setter&&) = delete;
//
//	Getter_Setter& operator=(const Getter_Setter&) = delete;
//	Getter_Setter& operator=(Getter_Setter&&) = delete;
//
//	Getter_Setter(_Parent* p, mem_func_get_t get, mem_func_set_t set)
//		: m_p(p)
//		, m_getter(get)
//		, m_setter(set)
//	{
//	}
//
//	operator _Typ() const { return (m_p->*m_getter)(); }
//
//	auto& operator=(const _Typ& t) { (m_p->*m_setter)(t); return *this; }
//	auto& operator=(_Typ&& t) { (m_p->*m_setter)(std::move(t));  return *this; }
//
//	auto& operator()(const _Typ& t) { (m_p->*m_setter)(t); return *m_p; }
//	auto& operator()(_Typ&& t) { (m_p->*m_setter)(std::move(t));  return *m_p; }
//
// private:
//	_Parent* m_p;
//
//	mem_func_get_t m_getter;
//	mem_func_set_t m_setter;
//};
//
//
// template<typename _Typ, typename _Parent>
// class Getter
//{
//	using mem_func_t = const _Typ& (_Parent::*)();
//
// public:
//	Getter() = delete;
//
//	Getter(const Getter&) = default;
//	Getter(Getter&&) = default;
//
//	Getter& operator=(const Getter&) = delete;
//	Getter& operator=(Getter&&) = delete;
//
//	Getter(_Parent* par, mem_func_t f)
//		: m_parent(par)
//		, m_getter(f)
//	{
//	}
//
//	operator const _Typ&() const { return (m_parent->*m_getter)(); }
//
// private:
//	_Parent* m_parent;
//	mem_func_t m_getter;
//};
//
// template<typename _Typ, typename _Parent>
// class Setter
//{
//	using mem_func_t = void (_Parent::*)(const _Typ&);
//
// public:
//	Setter() = delete;
//
//	Setter(const Setter&) = default;
//	Setter(Setter&&) = default;
//
//	Setter& operator=(const Setter&) = delete;
//	Setter& operator=(Setter&&) = delete;
//
//	Setter(_Parent* par, mem_func_t f)
//		: m_parent(par)
//		, m_getter(f)
//	{
//	}
//
//	operator const _Typ& () const { return (m_parent->*m_getter)(); }
//
// private:
//	_Parent* m_parent;
//	mem_func_t m_getter;
//};
