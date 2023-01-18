#include <core/memory.h>

namespace porte {

void *AllocAligned(size_t size) {
//#if defined(PORTE_HAVE__ALIGNED_MALLOC)
    return _aligned_malloc(size, PORTE_L1_CACHE_LINE_SIZE);
//#elif defined(PORTE_HAVE_POSIX_MEMALIGN)
//    void *ptr;
//    if (posix_memalign(&ptr, PBRT_L1_CACHE_LINE_SIZE, size) != 0) ptr = nullptr;
//    return ptr;
//#else
//    return memalign(PORTE_L1_CACHE_LINE_SIZE, size);
//#endif
}

void FreeAligned(void *ptr) {
    if (!ptr) return;
//#if defined(PORTE_HAVE__ALIGNED_MALLOC)
    _aligned_free(ptr);
//#else
//    free(ptr);
//#endif
}

}  // namespace porte
