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

	enum Type { BOOL, BYTE, WORD, DWORD, CHAR, INT, DINT, REAL, MAX };
	static constexpr std::array<size_t, MAX> TYPE_SIZE = { 1, 8, 16, 32, 8, 16, 32, 32 };

	basic_VarSeq() = default;

	const auto& db() const noexcept { return m_db; }
	void db(int db) noexcept { m_db = db; }

	using vec_t::begin;
	using vec_t::end;
	using vec_t::front;
	using vec_t::back;
	using vec_t::empty;
	using vec_t::size;
	using vec_t::operator[];
	using vec_t::emplace_back;
	using vec_t::push_back;

private:
	int m_db;
};

template<typename Impl>
class EVarInfo
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	EVarInfo() = default;

	size_t total_byte_size() const noexcept
	{
		return std::accumulate(underlying()->begin(), underlying()->end(), 0u, [](size_t num, const auto& i) { return num + i.byte_size(); });
	}

private:

};

template<typename Impl>
class EVarByteArray
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

	struct _LoopInt { size_t val : 3; };

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

		_LoopInt bool_skip{ 0 };	// Bools are stored in order in 1 byte
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

template<typename Impl>
class EVarParser
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	EVarParser() = default;

	void parse(std::string_view message)
	{
		Parser p;
		p.data(message);

		if (const auto num = p.get_num<unsigned int>('!'); num.has_value())
			underlying()->db(num.value());
		else
			throw Logger("Db not found in message.");

		while (!p.at_end())
		{
			Variable::Type typ;
			if (const auto res = p.get_num<unsigned int>('_'); res.has_value())
				typ = static_cast<Variable::Type>(res.value());
			else
				throw Logger("Variable type missing.");


			underlying()->emplace_back(typ);

			switch (typ)
			{
			case Variable::BOOL:
			case Variable::CHAR:
			case Variable::BYTE:
				underlying()->back().template fill_var<int8_t>(_get_val_<int8_t>(p));
				break;

			case Variable::INT:
			case Variable::WORD:
				underlying()->back().template fill_var<int16_t>(_get_val_<int16_t>(p));
				break;

			case Variable::DINT:
			case Variable::DWORD:
				underlying()->back().template fill_var<int32_t>(_get_val_<int32_t>(p));
				break;

			case Variable::REAL:
				underlying()->back().template fill_var<float>(_get_val_<float>(p));
				break;

			default:
				throw Logger("Undefinied type.");
				break;
			}
		}
	}

private:
	template<typename T>
	T _get_val_(Parser &p)
	{
		if (const auto res = p.get_num<T>('!'); res.has_value())
			return res.value();
		else
			throw Logger("Variable value missing or value too large.");
	}
};


using VarSeq = basic_VarSeq<Variable, EVarParser, EVarInfo, EVarByteArray>;