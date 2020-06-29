#pragma once

#include "Includes.h"
#include "VariableSequence.h"

class JSONValue
{
public:
	explicit JSONValue(rj::Value &val)
		: m_val(val)
	{
	}

	template<typename... _T>
	auto var(const char *first, _T &&... names) -> JSONValue
	{
		return var(first).var(std::forward<_T>(names)...);
	}

	auto var(const char *name) -> JSONValue
	{
		if (auto mem = m_val.FindMember(name); mem != m_val.MemberEnd())
			return JSONValue(mem->value);

		throw std::runtime_error(std::string("Member \"") + name + "\" not found.");
	}

	auto safe_var(const char *name) -> std::optional<JSONValue>
	{
		if (auto mem = m_val.FindMember(name); mem != m_val.MemberEnd())
			return JSONValue(mem->value);

		return std::nullopt;
	}

	[[nodiscard]] auto string() const -> const char *
	{
		if (m_val.IsString())
			return m_val.GetString();

		throw std::runtime_error("String at JSON element not found.");
	}

	template<typename Num>
	auto num() const -> Num
	{
		return guarded_get(str_to_num<Num>(string()), "Number at JSON element not found.");
	}

	[[nodiscard]] auto data() const noexcept -> const auto & { return m_val; }
	auto			   data() noexcept -> auto & { return m_val; }

	[[nodiscard]] auto begin() const noexcept { return m_val.MemberBegin(); }
	auto			   begin() noexcept { return m_val.MemberBegin(); }

	[[nodiscard]] auto end() const noexcept { return m_val.MemberEnd(); }
	auto			   end() noexcept { return m_val.MemberEnd(); }

private:
	rj::Value &m_val;
};

class JSONRoot
{
public:
	JSONRoot()			  = default;
	JSONRoot(JSONRoot &&) = default;

	JSONRoot(const JSONRoot &) = delete;

	explicit JSONRoot(std::string_view str) { from_string(str); }
	explicit JSONRoot(rj::Document &&doc)
		: m_doc(std::move(doc))
	{
	}

	void from_string(std::string_view str)
	{
		if (m_doc.Parse<rj::kParseNumbersAsStringsFlag>(str.data(), str.size()).HasParseError())
			throw std::runtime_error(rj::GetParseError_En(m_doc.GetParseError()));
	}

	[[nodiscard]] auto to_string() const -> std::string
	{
		rj::StringBuffer			 buffer;
		rj::Writer<decltype(buffer)> w(buffer);
		m_doc.Accept(w);

		return buffer.GetString();
	}

	template<typename... _T>
	auto var(const char *first, _T &&... names) 
	{
		return JSONValue(m_doc).var(first, std::forward<_T>(names)...);
	}

	auto allocator() noexcept -> auto & { return m_doc.GetAllocator(); }

	[[nodiscard]] auto begin() const noexcept { return m_doc.MemberBegin(); }
	[[nodiscard]] auto begin() noexcept { return m_doc.MemberBegin(); }

	[[nodiscard]] auto end() const noexcept { return m_doc.MemberEnd(); }
	[[nodiscard]] auto end() noexcept { return m_doc.MemberEnd(); }

private:
	rj::Document m_doc;
};
