#include <gbGraphics/detail/VulkanMemoryAllocator.hpp>

#if _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4127 4324)
#endif

#define VMA_IMPLEMENTATION
//#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
//#define VMA_DEBUG_MARGIN 16
//#define VMA_DEBUG_DETECT_CORRUPTION 1
#include <vk_mem_alloc.h>

#if _MSC_VER
#pragma warning(pop)
#endif


namespace GHULBUS_GRAPHICS_NAMESPACE::detail
{
}
