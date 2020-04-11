#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"
#include "utility.h"

template<typename Impl>
class EDebugHandler : public crtp<Impl, EDebugHandler>
{
public:
	EDebugHandler() = default;

protected:
	void _handle_debug(std::string_view debug)
	{
		g_log.write(Logger::Catagory::INFO) << "Debug message: " << debug;
	}

};

template<typename Impl>
class EDataHandler : public crtp<Impl, EDataHandler>
{
public:
	EDataHandler() = default;

	template<typename... T>
	auto& get_var(const char* first, T&&... names)
	{
		auto* level = &_get_member_(&m_d, first);
		((level = &_get_member_(level, names)), ...);
		return *level;
	}

	const auto& data() const noexcept { return m_d; }
	auto&& give_data() noexcept { return std::move(m_d); }

protected:
	void _handle_data(std::string_view data)
	{
		if (m_d.Parse<rj::kParseNumbersAsStringsFlag>(data.data(), data.size()).HasParseError())
			throw std::runtime_error(rj::GetParseError_En(m_d.GetParseError()));
	}

private:
	rj::Document m_d;


	auto& _get_member_(rj::Value* v, const char* name)
	{
		if (auto mem = v->FindMember(name); mem != v->MemberEnd())
			return mem->value;
		else
			throw std::runtime_error(std::string("Member ") + name + " not found.");
	}
};


class ResponseHandler
	: public EDebugHandler<ResponseHandler>
	, public EDataHandler<ResponseHandler>
{
public:
	enum HeaderList { START, DEBUG, DATA, END };
	static constexpr std::array<std::string_view, 4> HEADERS = { "#START\n", "#DEBUG\n", "#DATA\n", "#END" };

	ResponseHandler() = default;

	void go_through_content(std::string_view message)
	{
		g_log.write(Logger::Catagory::INFO) << "Checking message contents with the size of " << message.size();

		Parser parser;
		parser.data(message);

		g_log.write(Logger::Catagory::INFO, "Checking start");
		_check_start_(parser);

		g_log.write(Logger::Catagory::INFO, "Handling debug");
		_check_debug_(parser);

		g_log.write(Logger::Catagory::INFO, "Handling data");
		_check_data_(parser);

		g_log.write(Logger::Catagory::INFO, "Checking end");
		_check_end_(parser);
	}

private:
	void _check_start_(Parser& p)
	{
		if (!p.is_same(HEADERS[START]))
			throw std::runtime_error("Missing #START.");
	}

	void _check_debug_(Parser& p)
	{
		if (p.is_same(HEADERS[DEBUG]))
			this->_handle_debug(guarded_get(p.get_until('#'), "Missing #DEBUG message."));
	}

	void _check_data_(Parser& p)
	{
		if (!p.is_same(HEADERS[DATA]))
			throw std::runtime_error("Missing #DATA.");
		this->_handle_data(guarded_get(p.get_until('#'), "Missing #DATA message."));
	}

	void _check_end_(Parser& p)
	{
		if (!p.is_same(HEADERS[END]))
			throw std::runtime_error("Missing #END.");
	}
};