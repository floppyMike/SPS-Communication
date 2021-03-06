#pragma once

#include "Includes.h"
#include "Logging.h"
#include "VariableSequence.h"
#include "JSON.h"

class Interpreter
{
public:
	Interpreter() = default;

	auto prepare_seqs(JSONValue settings) -> void
	{
		// Get const and mutable from json
		for (auto &i : settings)
			if (::strcmp(i.name.GetString(), "const") == 0)
				m_seqs[DB_Type::CONST].db(JSONValue(i.value).num<int>());
			else if (::strcmp(i.name.GetString(), "mutable") == 0)
				m_seqs[DB_Type::MUTABLE].db(JSONValue(i.value).num<int>());
			else
				g_log.write(Logger::Catagory::ERR) << "Undefined option " << i.name.GetString() << " found.";
	}
	auto prepare_seqs() -> void
	{
		// Set default const and mutable
		m_seqs[DB_Type::CONST].db(1);
		m_seqs[DB_Type::MUTABLE].db(2);
	}

	auto prepare_vars(std::string_view filename) -> void
	{
		std::ifstream file_in(filename.data(), std::ios::in);

		// Ignore till section
		while (file_in.ignore(std::numeric_limits<std::streamsize>::max(), '#'))
		{
			std::string t;
			std::getline(file_in, t);

			// Match section
			if (t == "constant")
				m_seqs[DB_Type::CONST] = _get_vars_(file_in);
			else if (t == "mutable")
				m_seqs[DB_Type::MUTABLE] = _get_vars_(file_in);
		}
	}

	// !!! CONSTANT is ignored !!!
	auto fill_vars(JSONValue data_inner) -> void
	{
		for (auto &iter_seq : m_seqs[DB_Type::MUTABLE])
		{
			bool exists_f = false;

			// Fill variable with json value
			for (auto &member : data_inner)
				if (iter_seq.name() == member.name.GetString())
				{
					exists_f = true;
					iter_seq.fill_var(JSONValue(member.value).string());
					break;
				}

			if (!exists_f)
				g_log.write(Logger::Catagory::WARN) << "Variable \"" << iter_seq.name() << "\" not found";
		}
	}

	auto give_seqs() noexcept -> VariableSequences && { return std::move(m_seqs); }

private:
	VariableSequences m_seqs;

	static auto _str_to_type_(std::string_view str) -> Variable::Type
	{
		// Match string to array of defined types
		for (size_t i = 0; i < Variable::Type::MAX; ++i)
			if (str == Variable::TYPE_STR[i])
				return static_cast<Variable::Type>(i);

		throw std::runtime_error(std::string("Unrecognised type: ").append(str));
	}

	static auto _get_vars_(std::istream &in) -> std::vector<Variable>
	{
		std::vector<Variable> data;

		// Check if variable name available
		for (std::string var_name; in.peek() != '\n' && std::getline(in, var_name, ':');)
		{
			// Check if nothing available further
			if (!in)
			{
				g_log.write(Logger::Catagory::ERR)
					<< "Variable \"" << var_name << "\" has not type. -> Using only previous variables.";
				return data;
			}

			// Get type
			std::string type_str;
			std::getline(in, type_str);
			Variable::Type type = _str_to_type_(type_str);

			data.emplace_back(var_name, type);
		}

		return data;
	}
};
