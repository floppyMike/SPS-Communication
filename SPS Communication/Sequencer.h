#pragma once

#include "Includes.h"

enum DB_Type { REMOTE, LOCAL, MAX };

template<typename VarSeq>
using VariableSequences = std::array<VarSeq, MAX>;

template<typename VarSeq, template<typename> class... Ex>
class Sequencer : public Ex<Sequencer<VarSeq, Ex...>>...
{
public:
	Sequencer() = default;

	Sequencer(int var, int perm)
		: m_seqs{ var, perm }
	{
	}

	void push_var(std::string_view var, std::string_view val)
	{
		Parser p;
		p.data(var);

		if (const auto typ = _get_db_type_(p); typ.has_value())
		{
			_prepare_var_(p, m_seqs[typ.value()]);

			//Get loc for key
			this->_include_pos(typ.value(), guarded_get(p.get_num<size_t>(), "Missing index at variable."));

			m_seqs[typ.value()].back().fill_var(val);
		}
	}

	auto&& give() noexcept
	{
		this->_sort(m_seqs);
		return std::move(m_seqs);
	}

private:
	std::vector<size_t> m_key[MAX];
	VariableSequences<VarSeq> m_seqs;

	std::optional<DB_Type> _get_db_type_(Parser& p)
	{
		const auto db_num = guarded_get(p.get_num<int>('_'), "Db number isn't readable.");

		if (db_num == m_seqs[REMOTE].db())
			return REMOTE;
		else if (db_num == m_seqs[LOCAL].db())
			return LOCAL;
		else
		{
			g_log.write("Non existant db given: " + std::to_string(db_num) + " -> Ignoring.");
			return std::nullopt;
		}
	}

	void _prepare_var_(Parser& p, VarSeq& v)
	{
		const auto variable_type = guarded_get(p.get_until('_'), "Couldn't read variable type.");

		using var_t = typename VarSeq::var_t;
		bool was_used = false;

		for (auto iter_str = var_t::TYPE_STR.begin(); iter_str != var_t::TYPE_STR.end(); ++iter_str)
			if (*iter_str == variable_type)
			{
				v.emplace_back(static_cast<typename var_t::Type>(std::distance(var_t::TYPE_STR.begin(), iter_str)));
				was_used = true;
				break;
			}

		if (!was_used)
			throw Logger("Unknown variable type.");
	}
};

template<typename Impl>
class EKeyedSorter
{
public:
	EKeyedSorter() = default;

protected:
	void _include_pos(DB_Type db, size_t pos)
	{
		m_key[db].emplace_back(pos);
	}

	template<typename VarSeqs>
	auto& _sort(VarSeqs& seqs)
	{
		for (auto [iter_seq, iter_key] = std::pair(seqs.begin(), m_key); iter_seq != seqs.end(); ++iter_seq, ++iter_key)
			_sort_seq_(*iter_seq, *iter_key);

		return seqs;
	}

private:
	std::vector<size_t> m_key[MAX];


	template<typename _Seq>
	void _sort_seq_(_Seq& seq, std::vector<size_t>& key)
	{
		//Allocate and setup variables
		std::vector<typename _Seq::var_t> dat;
		dat.reserve(seq.size());
		for (const auto& i : seq)
			dat.emplace_back(i.type());

		//Sort values
		for (auto [iter_key, iter_val] = std::pair(key.begin(), seq.begin()); iter_key != key.end(); ++iter_key, ++iter_val)
		{
			if (*iter_key >= dat.size() - 1)
				throw Logger("Indexes are wrong.");

			dat[*iter_key] = *iter_val;
		}

		seq = std::move(dat);
	}
};
