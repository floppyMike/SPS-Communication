#pragma once
#include "Includes.h"

class Logger
{
public:
	static constexpr std::string_view LOG_FILE = "log.txt";

	Logger()
	{
		std::ofstream(LOG_FILE.data());
	}
	Logger(std::string_view message)
		: m_buffer(message)
	{
	}

	Logger(const Logger&) = delete;
	Logger(Logger&&) = delete;

	~Logger()
	{
		std::ofstream file(LOG_FILE.data(), std::ios::out | std::ios::binary | std::ios::app);
		file.write(m_buffer.data(), m_buffer.size());
	}

	void initiate(std::string_view what)
	{
		std::scoped_lock lock(m_mutex);
		_write_noflush_("Initiating " + std::string(what) + "...");
	}

	void complete()
	{
		write("Done\n");
	}

	template<typename Func>
	void log_func(std::string_view what, const Func& f)
	{
		initiate(what);
		f();
		complete();
	}

	void seperate()
	{
		write("\n----------------------------------------\n\n");
	}

	template<typename Iter>
	Logger& write(Iter begin, Iter end)
	{
		return write(&*begin, std::distance(begin, end));
	}

	Logger& write(std::string_view val)
	{
		return write(val.data(), val.size());
	}

	Logger& write(const char* str, size_t amount)
	{
		std::scoped_lock lock(m_mutex);
		_write_noflush_(str, amount);
		if (m_buffer.size() >= 1024)
			_flush_();

		return *this;
	}

private:
	std::mutex m_mutex;
	std::string m_buffer;


	void _write_noflush_(const char* str, size_t amount)
	{
		std::clog.write(str, amount);
		m_buffer.append(str, amount);
	}
	void _write_noflush_(std::string_view str)
	{
		std::clog << str;
		m_buffer.append(str);
	}

	void _flush_()
	{
		std::ofstream file(LOG_FILE.data(), std::ios::out | std::ios::binary);
		file.write(m_buffer.data(), m_buffer.size());
		m_buffer.clear();
	}
};

namespace
{
	Logger g_log;
}