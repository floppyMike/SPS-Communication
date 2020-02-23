#pragma once

#include "Includes.h"

template<typename T, template<typename> class crtpType>
struct crtp
{
	T* underlying() noexcept { return static_cast<T*>(this); }
	const T* underlying() const noexcept { return static_cast<const T*>(this); }

private:
	crtp() = default;
	friend crtpType<T>;
};
