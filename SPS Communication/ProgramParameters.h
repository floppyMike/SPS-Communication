#pragma once

#include "Includes.h"

enum ParaType
{
	LOCATION, SPS_PORT, HOST_SERVER, MAX
};

class ProgrammParameters : std::vector<std::string_view>
{
public:
	using vec_t = std::vector<std::string_view>;

	ProgrammParameters() = default;

	using vec_t::operator[];

	//template<typename _Array>
	//bool init(int arc, char** argv, const _Array& opt, int must = 0)
	//{
	//	m_para_opt.resize(std::size(opt) + must);

	//	auto iter_req = m_para_opt.begin();
	//	for (char** iter = argv, end = argv + arc; iter != end; ++iter)
	//		if (auto res = std::find(std::begin(opt), std::end(opt), *iter); res != std::end(opt))
	//			m_para_opt[must + std::distance(std::begin(opt), res)] = *iter;
	//		else
	//		{
	//			if (iter_req >= m_para_opt.begin() + must)
	//				return false;

	//			*(iter_req++) = *iter;
	//		}
	//}

	void init(int arc, char** argv)
	{
		this->assign(argv, argv + arc);
	}
};

namespace
{
	ProgrammParameters g_para;
}