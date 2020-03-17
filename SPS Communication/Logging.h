#pragma once
#include "Includes.h"

class Logger
{
	enum class _Color_
	{
		WHITE, GREEN, YELLOW, ORANGE, RED
	};

public:
	class _Stream_
	{
	public:
		_Stream_(Logger& log)
			: m_log(&log)
		{
		}

		~_Stream_()
		{
			m_log->_write_buffer_(m_s.str());
			m_log->_write_buffer_("\n");
		}

		template<typename T>
		auto& operator<<(const T& v)
		{
			m_s << v;
			return *this;
		}

	private:
		std::stringstream m_s;
		Logger* m_log;
	};

	enum class Catagory { INFO, WARN, ERR, FATAL };

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

	auto write(Catagory c)
	{
		_write_time_();
		_write_catagory_(c);

		return _Stream_(*this);
	}

	void seperate()
	{
		write(Catagory::INFO, "\n\n----------------------------------------\n");
	}

	Logger& write(Catagory c, std::string_view val)
	{
		return write(c, val.data(), val.size());
	}

	Logger& write(Catagory c, const char* str, size_t amount)
	{
		_write_time_();
		_write_catagory_(c);

		_write_buffer_(str, amount);
		_write_buffer_("\n");

		if (m_buffer.size() >= 1024)
			_flush_();

		return *this;
	}

private:
	std::string m_buffer;

	void _write_time_()
	{
		const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		char buf[21];
		tm time;
		gmtime_s(&time, &t);
		std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S ", &time);

		_write_buffer_(buf, sizeof buf);
	}

	void _write_catagory_(Catagory c)
	{
		switch (c)
		{
		case Catagory::INFO:	_write_buffer_("[INFO] ", _Color_::GREEN); break;
		case Catagory::WARN:	_write_buffer_("[WARN] ", _Color_::YELLOW); break;
		case Catagory::ERR:		_write_buffer_("[ERROR] ", _Color_::ORANGE); break;
		case Catagory::FATAL:	_write_buffer_("[FATAL] ", _Color_::RED); break;
		default: break;
		}
	}

	void _write_buffer_(std::string_view s, _Color_ col = _Color_::WHITE)
	{
		_write_buffer_(s.data(), s.size(), col);
	}

	void _write_buffer_(const char* str, size_t amount, _Color_ col = _Color_::WHITE)
	{
		m_buffer.append(str, amount);

		switch (col)
		{
		case Logger::_Color_::WHITE:	std::clog.write(str, amount);									break;
		case Logger::_Color_::GREEN:	std::clog << "\x1B[92m" + std::string(str, amount) + "\033[m";	break;
		case Logger::_Color_::YELLOW:	std::clog << "\x1B[93m" + std::string(str, amount) + "\033[m";	break;
		case Logger::_Color_::ORANGE:	std::clog << "\x1B[95m" + std::string(str, amount) + "\033[m";	break;
		case Logger::_Color_::RED:		std::clog << "\x1B[91m" + std::string(str, amount) + "\033[m";	break;
		default: break;
		}
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