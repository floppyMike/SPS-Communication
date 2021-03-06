#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"
#include "Variable.h"

class VarSequence : std::vector<Variable>
{
public:
	using vec_t = std::vector<Variable>;

	VarSequence() = default;

	VarSequence(int db, std::vector<Variable> &&v)
		: std::vector<Variable>(std::move(v))
		, m_db(db)
	{
	}

	[[nodiscard]] auto total_byte_size() const noexcept -> size_t
	{
		return std::accumulate(begin(), end(), 0U, [](size_t num, const auto &i) { return num + i.byte_size(); });
	}

	using vec_t::assign;
	using vec_t::back;
	using vec_t::begin;
	using vec_t::emplace_back;
	using vec_t::empty;
	using vec_t::end;
	using vec_t::front;
	using vec_t::push_back;
	using vec_t::size;

	using vec_t::operator[];
	using vec_t::operator=;

	friend auto operator<<(std::ostream &o, const VarSequence &v) -> std::ostream &
	{
		o << "DB: " << v.m_db << '\n';

		for (const auto &i : v) o << i << '\n';

		return o;
	}

	auto operator==(const VarSequence &s) const noexcept -> bool
	{
		return std::equal(this->begin(), this->end(), s.begin());
	}

	[[nodiscard]] auto db() const noexcept -> const auto & { return m_db; }
	auto			   db(int db) noexcept -> auto &
	{
		m_db = db;
		return *this;
	}

private:
	int m_db = 0;
};

enum DB_Type
{
	MUTABLE,
	CONST,
	DB_MAX
};
using VariableSequences = std::array<VarSequence, DB_MAX>;
