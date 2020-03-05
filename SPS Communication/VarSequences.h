#pragma once

#include "Includes.h"

//template<typename VarSeq>
//class DataSequences
//{
//public:
//	enum DB_Type { REMOTE, LOCAL, MAX };
//
//	DataSequences() = default;
//	DataSequences(int var, int perm)
//		: m_db{ var, perm }
//	{
//	}
//
//	void push_var(std::string_view var, std::string_view val, std::vector<size_t>(&key)[MAX])
//	{
//		Parser p;
//		p.data(var);
//
//		if (const auto typ = _get_db_type_(p); typ.has_value())
//		{
//			_place_var_(p, m_seqs[typ.value()]);
//
//			//Get loc for key
//			if (const auto val = p.get_num<size_t>(); val.has_value())
//				key[typ.value()].emplace_back(val.value());
//			else
//				throw Logger("Missing index at variable.");
//
//			m_seqs[typ.value()].back().fill_var(val);
//		}
//	}
//
//	//void sort(std::vector<size_t>(&key)[MAX])
//	//{
//	//	for (DB_Type i = REMOTE; i < MAX; ++i)
//	//		std::sort(m_seqs[i].begin(), m_seqs[i].end(), [&key](const VarSeq& v1, const VarSeq& v2) { return  })
//	//}
//
//	auto db(DB_Type t) const noexcept { return m_db[t]; }
//
//	const auto& sequence(DB_Type t) const noexcept { return m_seqs[t]; }
//	auto& sequence(DB_Type t) noexcept { return m_seqs[t]; }
//
//private:
//	std::array<VarSeq, MAX> m_seqs;
//	int m_db[MAX];
//
//
//	std::optional<DB_Type> _get_db_type_(Parser& p)
//	{
//		DB_Type typ;
//		if (const auto val = p.get_num<int>('_'); val.has_value())
//			if (val.value() == m_db[REMOTE])
//				typ = REMOTE;
//			else if (val.value() == m_db[LOCAL])
//				typ = LOCAL;
//			else
//			{
//				g_log.write("Non existant db given: " + std::to_string(val.value()) + " -> Ignoring.");
//				return std::nullopt;
//			}
//		else
//			throw Logger("Db number isn't readable.");
//
//		return typ;
//	}
//
//	void _place_var_(Parser& p, VarSeq& v)
//	{
//		if (const auto val = p.get_until('_'); val.has_value())
//		{
//			using var_t = typename VarSeq::var_t;
//			bool was_used = false;
//
//			for (auto iter_str = var_t::TYPE_STR.begin(); iter_str != var_t::TYPE_STR.end(); ++iter_str)
//				if (*iter_str == val.value())
//				{
//					v.emplace_back(static_cast<typename var_t::Type>(std::distance(var_t::TYPE_STR.begin(), iter_str)));
//					was_used = true;
//					break;
//				}
//
//			if (!was_used)
//				throw Logger("Unknown variable type.");
//		}
//		else
//			throw Logger("Couldn't read variable type.");
//	}
//};

enum DB_Type { REMOTE, LOCAL, MAX };

template<typename VarSeq>
using VariableSequences = std::array<VarSeq, MAX>;

template<typename VarSeq>
class Intepreter
{
public:
	Intepreter() = default;

	Intepreter(int var, int perm)
		: m_seqs{ var, perm }
	{
	}

	void push_var(std::string_view var, std::string_view val)
	{
		Parser p;
		p.data(var);

		if (const auto typ = _get_db_type_(p); typ.has_value())
		{
			_place_var_(p, m_seqs[typ.value()]);

			//Get loc for key
			if (const auto val = p.get_num<size_t>(); val.has_value())
				m_key[typ.value()].emplace_back(val.value());
			else
				throw Logger("Missing index at variable.");

			m_seqs[typ.value()].back().fill_var(val);
		}
	}

	auto&& give() noexcept
	{
		for (auto [iter_seq, iter_key] = std::pair(m_seqs.begin(), m_key); iter_seq != m_seqs.end(); ++iter_seq, ++iter_key)
			iter_seq->sort<VarSeq::var_t>(*iter_key);

		return std::move(m_seqs);
	}

private:
	std::vector<size_t> m_key[MAX];
	VariableSequences<VarSeq> m_seqs;

	std::optional<DB_Type> _get_db_type_(Parser& p)
	{
		DB_Type typ;
		if (const auto val = p.get_num<int>('_'); val.has_value())
			if (val.value() == m_seqs[REMOTE].db())
				typ = REMOTE;
			else if (val.value() == m_seqs[LOCAL].db())
				typ = LOCAL;
			else
			{
				g_log.write("Non existant db given: " + std::to_string(val.value()) + " -> Ignoring.");
				return std::nullopt;
			}
		else
			throw Logger("Db number isn't readable.");

		return typ;
	}

	void _place_var_(Parser& p, VarSeq& v)
	{
		if (const auto val = p.get_until('_'); val.has_value())
		{
			using var_t = typename VarSeq::var_t;
			bool was_used = false;

			for (auto iter_str = var_t::TYPE_STR.begin(); iter_str != var_t::TYPE_STR.end(); ++iter_str)
				if (*iter_str == val.value())
				{
					v.emplace_back(static_cast<typename var_t::Type>(std::distance(var_t::TYPE_STR.begin(), iter_str)));
					was_used = true;
					break;
				}

			if (!was_used)
				throw Logger("Unknown variable type.");
		}
		else
			throw Logger("Couldn't read variable type.");
	}
};

