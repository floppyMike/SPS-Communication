#pragma once

#include "Includes.h"
#include "VariableSequence.h"


class ByteArrayConverter
{
	struct _LoopInt_
	{
		size_t val : 3;
	};

public:
	ByteArrayConverter() = default;

	static auto from_byte_array(VarSequence &seq, const std::vector<uint8_t> &bytes)
	{
		_LoopInt_ bool_skip{ 0 }; // Bools are stored in order in 1 byte
		for (auto [iter_byte, iter_seq] = std::pair(bytes.begin(), seq.begin()); iter_seq != seq.end(); ++iter_seq)
			if (iter_seq->type() == Variable::BOOL)
			{
				if (iter_seq != seq.begin() && bool_skip.val == 0)
					++iter_seq;

				iter_seq->fill_var(static_cast<uint8_t>((*iter_byte >> bool_skip.val++) & 1U));
			}
			else
			{
				if (bool_skip.val != 0)
					++iter_byte, bool_skip.val = 0;

				if (!(iter_seq->byte_size() & 1U) && std::distance(bytes.begin(), iter_byte) & 1U)
					++iter_byte;

				const auto &type_size = Variable::TYPE_SIZE[iter_seq->type()];

				iter_seq->fill_var(std::vector(iter_byte, iter_byte + type_size));
				iter_byte += type_size;
			}
	}

	static auto to_byte_array(const VarSequence &seq)
	{
		std::vector<uint8_t> arr;

		_LoopInt_ bool_skip{ 0 }; // Bools are stored in order in 1 byte
		size_t	  was_byte = 0;	  // Bytes (BOOL, CHAR, BYTE) must be stored evenly
		for (auto iter_var = seq.begin(); iter_var != seq.end(); ++iter_var)
			if (iter_var->type() == Variable::BOOL)
			{
				if (bool_skip.val == 0)
					arr.emplace_back(), ++was_byte;

				arr.back() |= iter_var->data().front() << bool_skip.val++;
			}
			else
			{
				if (iter_var->byte_size() != 1)
				{
					if (was_byte & 1U)
						arr.emplace_back(), was_byte = 0;
				}
				else
					++was_byte;

				arr.insert(arr.end(), iter_var->data().begin(), iter_var->data().end());
				bool_skip.val = 0;
			}

		return arr;
	}
};
