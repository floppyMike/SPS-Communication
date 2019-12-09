#pragma once
#include "Includes.h"


template<template<typename> class... Ex>
class basic_RequestBuffer : std::vector<std::string>, public Ex<basic_RequestBuffer<Ex...>>...
{
public:
	basic_RequestBuffer() = default;

	void emplace_back()
	{

	}

private:
	std::mutex 
};


template<typename Impl>
class ERequester
{
	inline auto& pthis() noexcept { return *static_cast<Impl*>(this); }

public:
	ERequester()
		: m_handler()
	{
	}

private:
	std::thread m_handler;


	void _run_time_()
	{

	}
};


using RequestBuffer = basic_RequestBuffer<ERequester>;