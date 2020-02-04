#pragma once

#include "Includes.h"

template<typename Iter, typename... Arg>
bool safe_equal(Iter ptr, Iter must_ptr, size_t size, Arg&&... args)
{
	return std::distance(ptr, must_ptr) < size - 1 && std::equal(ptr, ptr + size - 1, std::forward<Arg>(args)...);
}