#pragma once

#include "Includes.h"

enum ParaType
{
	LOCATION,
	SPS_HOST,
	SPS_PORT,
	HOST_SERVER,
	MAX
};

class ProgrammParameters
{
public:
	ProgrammParameters() = default;

	const char *operator[](size_t i) { return m_arr[i]; }

	// template<typename _Array>
	// bool init(int arc, char** argv, const _Array& opt, int must = 0)
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

	void init(int arc, char **argv)
	{
		m_arr  = argv;
		m_size = arc;
	}

private:
	const char *const *m_arr;
	size_t			   m_size;
};

namespace
{
	ProgrammParameters g_para;
}
