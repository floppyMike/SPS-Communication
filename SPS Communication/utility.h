#pragma once

#include "Includes.h"

template<typename T, template<typename> class crtpType>
struct crtp
{
	T* underlying() noexcept { return static_cast<T*>(this); }
	const T* underlying() const noexcept { return static_cast<const T*>(this); }

private:
	crtp() = default;
	friend crtpType<T>;
};

template<typename T>
std::optional<T> str_to_num(std::string_view str) noexcept
{
	T temp;
	if (str.empty() || std::from_chars(str.data(), str.data() + str.size(), temp).ec != std::errc())
		return std::nullopt;
	return temp;
}

//template<typename _Typ, typename _Parent>
//class Getter_Setter
//{
//	using mem_func_set_t = void (_Parent::*)(const _Typ&);
//	using mem_func_get_t = const _Typ& (_Parent::*)();
//
//public:
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
//private:
//	_Parent* m_p;
//
//	mem_func_get_t m_getter;
//	mem_func_set_t m_setter;
//};
//
//
//template<typename _Typ, typename _Parent>
//class Getter
//{
//	using mem_func_t = const _Typ& (_Parent::*)();
//
//public:
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
//private:
//	_Parent* m_parent;
//	mem_func_t m_getter;
//};
//
//template<typename _Typ, typename _Parent>
//class Setter
//{
//	using mem_func_t = void (_Parent::*)(const _Typ&);
//
//public:
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
//private:
//	_Parent* m_parent;
//	mem_func_t m_getter;
//};
