#pragma once
#include "Includes.h"

class Logger
{
public:
	static constexpr std::string_view LOG_FILE = "log.txt";

	Logger() = default;
	Logger(std::string_view message)
		: m_buffer(message)
	{
	}

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

	void seperate()
	{
		write("\n----------------------------------------\n\n");
	}

	void write(std::string_view val)
	{
		std::scoped_lock lock(m_mutex);
		_write_noflush_(val);
		if (m_buffer.size() >= 1024)
		{
			std::ofstream file(LOG_FILE.data(), std::ios::out | std::ios::binary);
			file.write(m_buffer.data(), m_buffer.size());
			m_buffer.clear();
		}
	}

private:
	std::mutex m_mutex;
	std::string m_buffer;


	void _write_noflush_(std::string_view val)
	{
		std::clog << val;
		m_buffer += val;
	}
};

static Logger g_log;
