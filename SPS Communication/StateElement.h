#pragma once

#include "Includes.h"
#include "Logging.h"
#include "Parser.h"
#include "VariableSequence.h"

template<typename _VarSeq, template<typename> class... Ex>
class basic_StateElement : std::vector<_VarSeq>, public Ex<basic_StateElement<_VarSeq, Ex...>>...
{
	using vec_t = std::vector<_VarSeq>;

public:
	static constexpr std::string_view VAR_NAME = "[state]=>";

	using varseq_t = _VarSeq;

	basic_StateElement() = default;

	using vec_t::begin;
	using vec_t::end;
	using vec_t::front;
	using vec_t::back;
	using vec_t::operator[];
	using vec_t::emplace_back;
	using vec_t::push_back;
	
};

template<typename Impl>
class EStateParser
{
	const Impl* underlying() const noexcept { return static_cast<const Impl*>(this); }
	Impl* underlying() noexcept { return static_cast<Impl*>(this); }

public:
	EStateParser() = default;

	void parse(std::string_view message)
	{
		Parser p;
		p.data(message);

		if (!p.is_same("[state]=>"))
			throw Logger("State wasn't found.");

		while (!p.at_end())
		{
			underlying()->emplace_back();

			if (const auto str = p.get_until(std::array{ '|', '\n' }); str.has_value())
				underlying()->back().parse(str.value()),
				p.mov(1);
			else
				throw Logger("Couldn't read state message.");
		}
	}

};


using StateElement = basic_StateElement<VarSeq, EStateParser>;