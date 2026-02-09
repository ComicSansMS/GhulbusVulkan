#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_HOST_MEMORY_ALLOCATOR_GLOBAL_NEW_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_HOST_MEMORY_ALLOCATOR_GLOBAL_NEW_HPP

/** @file
*
* @brief Host Memory Allocator Global New.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/
#include <gbVk/HostMemoryAllocator.hpp>

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <cstddef>
#include <unordered_map>

namespace GHULBUS_VULKAN_NAMESPACE
{

class HostMemoryAllocator_GlobalNew final : public HostMemoryAllocator {
private:
    struct AllocationInfo {
        std::size_t size;
        std::size_t alignment;
        VkSystemAllocationScope scope;
    };
    std::unordered_map<void*, AllocationInfo> m_activeAllocations;
public:
    HostMemoryAllocator_GlobalNew();
    ~HostMemoryAllocator_GlobalNew() override;

    HostMemoryAllocator_GlobalNew(HostMemoryAllocator_GlobalNew const&) = delete;
    HostMemoryAllocator_GlobalNew& operator=(HostMemoryAllocator_GlobalNew const&) = delete;

    HostMemoryAllocator_GlobalNew(HostMemoryAllocator_GlobalNew&& rhs) = delete;
    HostMemoryAllocator_GlobalNew& operator=(HostMemoryAllocator_GlobalNew&& rhs) = delete;

    VkAllocationCallbacks getVkAllocationCallbacks() override;

    void* allocate(std::size_t size, std::size_t alignment, VkSystemAllocationScope allocationScope);
    void* reallocate(void* pOriginal, std::size_t size, std::size_t alignment, VkSystemAllocationScope allocationScope);
    void free(void* pMemory);
    void internalAllocationNotification(std::size_t size, VkInternalAllocationType allocationType,
                                        VkSystemAllocationScope allocationScope);
    void internalFreeNotification(std::size_t size, VkInternalAllocationType allocationType,
                                  VkSystemAllocationScope allocationScope);

};

}

#endif
