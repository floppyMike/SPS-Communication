#pragma once
#include "Includes.h"
#include "Server.h"


template<template<typename> class... Ex>
class basic_RequestBuffer : std::queue<std::string>, public Ex<basic_RequestBuffer<Ex...>>...
{
public:
	basic_RequestBuffer() = default;

	std::string poll()
	{
		std::scoped_lock lock(m_mutex);
		auto&& str = std::move(this->front());
		this->pop();
		return str;
	}

	void emplace(std::string&& str)
	{
		std::scoped_lock lock(m_mutex);
		this->emplace(std::move(str));
	}

private:
	std::mutex m_mutex;
};


template<typename Impl>
class ERequester
{
	//inline auto& pthis() noexcept { return *static_cast<Impl*>(this); }

	Impl* pthis = *static_cast<Impl*>(this);

public:
	ERequester()
		: m_handler(_run_time_, this)
	{
	}

private:
	std::thread m_handler;
	


	void _run_time_(asio::io_context& io)
	{
		auto next_request = std::chrono::steady_clock::now();

		while (true)
		{
			if (next_request <= std::chrono::steady_clock::now())
			{
				next_request = std::chrono::steady_clock::now();
				auto result = query<Session>(io, "localhost", "/data.txt");
				auto timeout = std::stoi(result.content.find());
			}
		}
	}
};


using RequestBuffer = basic_RequestBuffer<ERequester>;