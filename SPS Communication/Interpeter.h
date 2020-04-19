#pragma once

#include "Includes.h"
#include "Logging.h"
#include "VariableSequence.h"
#include "JSON.h"

class Interpreter
{
public:
	Interpreter() = default;

	auto&& interpret_json(JSONValue root_data, std::string_view varfile)
	{
		if (auto s = root_data.safe_var("settings"); s.has_value())
			prepare_seqs(s.value());
		else
			prepare_seqs();

		prepare_vars(varfile);

		if (auto s = root_data.safe_var("data"); s.has_value())
			fill_vars(s.value());

		return std::move(m_seqs);
	}

	void prepare_seqs(JSONValue settings)
	{
		for (auto& i : settings)
			if (::strcmp(i.name.GetString(), "const") == 0)
				m_seqs[DB_Type::CONST].db(JSONValue(i.value).num<int>());
			else if (::strcmp(i.name.GetString(), "mutable") == 0)
				m_seqs[DB_Type::MUTABLE].db(JSONValue(i.value).num<int>());
			else
				g_log.write(Logger::Catagory::ERR) << "Undefined option " << i.name.GetString() << " found.";
	}
	void prepare_seqs()
	{
		m_seqs[DB_Type::CONST].db(1);
		m_seqs[DB_Type::MUTABLE].db(2);
	}

	void prepare_vars(std::string_view filename)
	{
		std::ifstream file_in(filename.data(), std::ios::in);

		while (file_in.ignore(std::numeric_limits<std::streamsize>::max(), '#'))
		{
			std::string t;
			std::getline(file_in, t);

			if (t == "constant")
				m_seqs[DB_Type::CONST] = _get_vars_(file_in);
			else if (t == "mutable")
				m_seqs[DB_Type::MUTABLE] = _get_vars_(file_in);
		}
	}

	// !!! CONSTANT is ignored !!!
	void fill_vars(JSONValue data_inner)
	{
		for (auto iter_seq = m_seqs[DB_Type::MUTABLE].begin(); iter_seq != m_seqs[DB_Type::MUTABLE].end(); ++iter_seq)
		{
			bool exists_f = false;

			for (auto& member : data_inner)
				if (iter_seq->name() == member.name.GetString())
				{
					exists_f = true;
					iter_seq->fill_var(JSONValue(member.value).string());
					break;
				}

			if (!exists_f)
				g_log.write(Logger::Catagory::WARN) << "Variable \"" << iter_seq->name() << "\" not found";
		}
	}

	VariableSequences&& give_seqs() noexcept { return std::move(m_seqs); }

private:
	VariableSequences m_seqs;

	Variable::Type _str_to_type_(std::string_view str)
	{
		for (size_t i = 0; i < Variable::Type::MAX; ++i)
			if (str == Variable::TYPE_STR[i])
				return static_cast<Variable::Type>(i);

		throw std::runtime_error(std::string("Unrecognised type: ").append(str));
	}

	std::vector<Variable> _get_vars_(std::istream& in)
	{
		std::vector<Variable> data;

		for (std::string var_name; in.peek() != '\n' && std::getline(in, var_name, ':');)
		{
			//Check if nothing available further
			if (!in)
			{
				g_log.write(Logger::Catagory::ERR) << "Variable \"" << var_name << "\" has not type. -> Using only previous variables.";
				return data;
			}

			//Get type
			std::string type_str;
			std::getline(in, type_str);
			Variable::Type type = _str_to_type_(type_str);

			data.emplace_back(var_name, type);
		}

		return data;
	}
};
