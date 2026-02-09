#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_HOST_MEMORY_ALLOCATOR_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_HOST_MEMORY_ALLOCATOR_HPP

/** @file
*
* @brief Host Memory Allocator Interface.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <memory>

namespace GHULBUS_VULKAN_NAMESPACE
{

class HostMemoryAllocator {
protected:
    HostMemoryAllocator() = default;
public:
    virtual ~HostMemoryAllocator() = 0;

    HostMemoryAllocator(HostMemoryAllocator const&) = delete;
    HostMemoryAllocator& operator=(HostMemoryAllocator const&) = delete;

    HostMemoryAllocator(HostMemoryAllocator&& rhs) = delete;
    HostMemoryAllocator& operator=(HostMemoryAllocator&& rhs) = delete;

    virtual VkAllocationCallbacks getVkAllocationCallbacks() = 0;
};

}

#endif
