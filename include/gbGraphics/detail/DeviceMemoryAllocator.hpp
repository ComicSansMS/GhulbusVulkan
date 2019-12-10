#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_DEVICE_MEMORY_ALLOCATOR_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_DEVICE_MEMORY_ALLOCATOR_HPP

/** @file
*
* @brief Device Memory Allocator.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <vk_mem_alloc.h>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
namespace detail
{
class DeviceMemoryAllocator {
private:
    VmaAllocator m_allocator;
public:
    DeviceMemoryAllocator(GhulbusVulkan::Instance& instance, GhulbusVulkan::Device& device);

    ~DeviceMemoryAllocator();

    DeviceMemoryAllocator(DeviceMemoryAllocator const&) = delete;
    DeviceMemoryAllocator& operator=(DeviceMemoryAllocator const&) = delete;

    DeviceMemoryAllocator(DeviceMemoryAllocator&& rhs);
    DeviceMemoryAllocator& operator=(DeviceMemoryAllocator&& rhs);
};
}
}
#endif
