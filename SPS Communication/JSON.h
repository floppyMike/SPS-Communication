#pragma once

#include "Includes.h"
#include "VariableSequence.h"

class JSONValue
{
public:
	JSONValue(rj::Value& val)
		: m_val(val)
	{
	}

	template<typename... _T>
	auto var(const char* first, _T&&... names)
	{
		return var(first).var(std::forward<_T>(names)...);
	}

	auto var(const char* name)
	{
		if (auto mem = m_val.FindMember(name); mem != m_val.MemberEnd())
			return JSONValue(mem->value);
		else
			throw std::runtime_error(std::string("Member \"") + name + "\" not found.");
	}

	std::optional<JSONValue> safe_var(const char* name)
	{
		if (auto mem = m_val.FindMember(name); mem != m_val.MemberEnd())
			return JSONValue(mem->value);
		else
			return std::nullopt;
	}

	const char* string() const
	{
		if (m_val.IsString())
			return m_val.GetString();
		else
			throw std::runtime_error("String at JSON element not found.");
	}

	template<typename Num>
	Num num() const
	{
		return guarded_get(str_to_num<Num>(string()), "Number at JSON element not found.");
	}

	const auto& data() const noexcept { return m_val; }
	auto& data() noexcept { return m_val; }

	auto begin() const noexcept { return m_val.MemberBegin(); }
	auto begin() noexcept { return m_val.MemberBegin(); }

	auto end() const noexcept { return m_val.MemberEnd(); }
	auto end() noexcept { return m_val.MemberEnd(); }

private:
	rj::Value& m_val;
};


class JSONRoot
{
public:
	JSONRoot() = default;
	JSONRoot(JSONRoot&&) = default;

	JSONRoot(const JSONRoot&) = delete;

	JSONRoot(std::string_view str) { from_string(str); }
	JSONRoot(rj::Document&& doc) : m_doc(std::move(doc)) {}

	void from_string(std::string_view str)
	{
		if (m_doc.Parse<rj::kParseNumbersAsStringsFlag>(str.data(), str.size()).HasParseError())
			throw std::runtime_error(rj::GetParseError_En(m_doc.GetParseError()));
	}

	std::string to_string() const
	{
		rj::StringBuffer buffer;
		rj::Writer<decltype(buffer)> w(buffer);
		m_doc.Accept(w);

		return buffer.GetString();
	}

	template<typename... _T>
	auto var(const char* first, _T&&... names)
	{
		return JSONValue(m_doc).var(first, std::forward<_T>(names)...);
	}

	auto& allocator() noexcept { return m_doc.GetAllocator(); }

	auto begin() const noexcept { return m_doc.MemberBegin(); }
	auto begin() noexcept { return m_doc.MemberBegin(); }

	auto end() const noexcept { return m_doc.MemberEnd(); }
	auto end() noexcept { return m_doc.MemberEnd(); }

private:
	rj::Document m_doc;
};
