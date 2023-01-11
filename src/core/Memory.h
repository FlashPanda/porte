#pragma once

#include <cstddef>

namespace porte
{
	void* AllocAligned(size_t size);

	template <typename T>
	T* AllocAligned(size_t count)
	{
		return (T*)AllocAligned(count * sizeof(T));
	}

	void FreeAligned(void*);
}