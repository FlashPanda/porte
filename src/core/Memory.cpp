#include "Memory.h"
#include <cstdlib>

namespace panda
{
	void* AllocAligned(size_t size)
	{
		return _aligned_malloc(size, 64);
	}

	void FreeAligned(void* ptr)
	{
		if (!ptr) return;

		_aligned_free(ptr);
	}
}