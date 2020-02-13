#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"

template<template<typename> class... Ex>
class basic_AuthElement : public Ex<basic_AuthElement<Ex...>>...
{
public:
	basic_AuthElement() = default;

	const auto& code() const noexcept { return m_code; }
	void code(std::string_view str) noexcept { m_code = str; }

	operator std::string_view()
	{
		return m_code;
	}

private:
	std::string m_code;
};

template<typename Impl>
class EAuthParser
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	EAuthParser() = default;

	void parse(std::string_view message)
	{
		Parser p;
		p.data(message);

		if (!p.is_same("[authcode]=>"))
			throw Logger("Authcode variable not found.");

		if (const auto str = p.get_until('\n'); str.has_value())
			underlying()->code(str.value());
		else
			throw Logger("Coundn't read authcode.");
	}

private:

};

using AuthElement = basic_AuthElement<EAuthParser>;
