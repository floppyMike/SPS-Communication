#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"
#include "Variable.h"

auto& operator<<(std::ostream& o, const std::vector<uint8_t>& bytes)
{
	for (const auto& i : bytes)
		o << std::hex << +i << ' ';
	o.put('\n');

	return o;
}


class VarSequence : std::vector<Variable>
{
public:
	using vec_t = std::vector<Variable>;

	using vec_t::vec_t;

	VarSequence(int db)
		: m_db(db)
	{
	}

	size_t total_byte_size() const noexcept
	{
		return std::accumulate(begin(), end(), 0u, [](size_t num, const auto& i) { return num + i.byte_size(); });
	}

	const auto& db() const noexcept { return m_db; }
	auto& db(int db) noexcept { m_db = db; return *this; }

	using vec_t::assign;
	using vec_t::begin;
	using vec_t::end;
	using vec_t::front;
	using vec_t::back;
	using vec_t::empty;
	using vec_t::size;
	using vec_t::emplace_back;
	using vec_t::push_back;

	using vec_t::operator[];
	using vec_t::operator=;

	friend std::ostream& operator<<(std::ostream& o, const VarSequence& v)
	{
		o << "DB: " << v.m_db << '\n';

		for (const auto& i : v)
			o << i << '\n';

		return o;
	}

	bool operator==(const VarSequence& s) const noexcept
	{
		return std::equal(this->begin(), this->end(), s.begin());
	}

private:
	int m_db = 0;
};

class ByteArrayConverter
{
	struct _LoopInt_ { size_t val : 3; };

public:
	ByteArrayConverter() = default;

	void from_byte_array(VarSequence& seq, const std::vector<uint8_t>& bytes)
	{
		_LoopInt_ bool_skip{ 0 };	// Bools are stored in order in 1 byte
		for (auto [iter_byte, iter_seq] = std::pair(bytes.begin(), seq.begin()); iter_seq != seq.end(); ++iter_seq)
			if (iter_seq->type() == Variable::BOOL)
			{
				if (iter_seq != seq.begin() && bool_skip.val == 0)
					++iter_seq;

				iter_seq->fill_var(static_cast<uint8_t>((*iter_byte >> bool_skip.val++) & 1));
			}
			else
			{
				if (bool_skip.val != 0)
					++iter_byte,
					bool_skip.val = 0;

				if (!(iter_seq->byte_size() & 1) && std::distance(bytes.begin(), iter_byte) & 1)
					++iter_byte;

				const auto& type_size = Variable::TYPE_SIZE[iter_seq->type()];

				iter_seq->fill_var(std::vector(iter_byte, iter_byte + type_size));
				iter_byte += type_size;
			}
	}

	auto to_byte_array(const VarSequence& seq) const
	{
		std::vector<uint8_t> arr;

		_LoopInt_ bool_skip{ 0 };	// Bools are stored in order in 1 byte
		size_t was_byte = 0;		// Bytes (BOOL, CHAR, BYTE) must be stored evenly
		for (auto iter_var = seq.begin(); iter_var != seq.end(); ++iter_var)
			if (iter_var->type() == Variable::BOOL)
			{
				if (bool_skip.val == 0)
					arr.emplace_back(),
					++was_byte;

				arr.back() |= iter_var->data().front() << bool_skip.val++;
			}
			else
			{
				if (iter_var->byte_size() != 1)
				{
					if (was_byte & 1)
						arr.emplace_back(),
						was_byte = 0;
				}
				else
					++was_byte;

				arr.insert(arr.end(), iter_var->data().begin(), iter_var->data().end());
				bool_skip.val = 0;
			}

		return arr;
	}
};

enum DB_Type { MUTABLE, CONST, DB_MAX };
using VariableSequences = std::array<VarSequence, DB_MAX>;