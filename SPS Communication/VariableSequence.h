#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"
#include "Variable.h"

template<typename _Var, template<typename> class... Ex>
class basic_VarSeq : std::vector<_Var>, public Ex<basic_VarSeq<_Var, Ex...>>...
{
	using vec_t = std::vector<_Var>;

public:
	using var_t = _Var;

	basic_VarSeq() = default;
	basic_VarSeq(int db)
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
	using vec_t::operator[];
	using vec_t::operator=;
	using vec_t::emplace_back;
	using vec_t::push_back;

	friend std::ostream& operator<<(std::ostream& o, const basic_VarSeq& v)
	{
		o << "DB: " << v.m_db << '\n';

		for (const auto& i : v)
			o << i << '\n';

		return o;
	}

private:
	int m_db;
};

template<typename Impl>
class EVarByteArray
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

	struct _LoopInt_ { size_t val : 3; };

public:
	EVarByteArray() = default;

	void from_byte_array(const std::vector<uint8_t>& bytes)
	{
		if (underlying()->total_byte_size() != bytes.size())
			throw Logger("Too many or not enought bytes to fill out from SPS.");

		for (auto [iter_byte, iter_seq] = std::pair(bytes.begin(), underlying()->begin()); iter_seq != underlying()->end(); ++iter_seq)
		{
			const auto iter_byte_end = iter_byte + iter_seq->byte_size();
			iter_seq->fill_var({ iter_byte, iter_byte_end });
			iter_byte = iter_byte_end;
		}
	}

	auto to_byte_array() const
	{
		std::vector<uint8_t> arr;

		_LoopInt_ bool_skip{ 0 };	// Bools are stored in order in 1 byte
		size_t was_byte = 0;		// Bytes (BOOL, CHAR, BYTE) must be stored evenly
		for (auto iter_var = underlying()->begin(); iter_var != underlying()->end(); ++iter_var)
		{
			if (iter_var->type() == Impl::var_t::BOOL)
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
		}

		return arr;
	}
};

auto& operator<<(std::ostream& o, const std::vector<uint8_t>& bytes)
{
	for (const auto& i : bytes)
		o << std::hex << +i << ' ';
	o.put('\n');

	return o;
}
