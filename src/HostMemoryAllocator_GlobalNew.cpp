#include <gbVk/HostMemoryAllocator_GlobalNew.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <cstring>
#include <new>
#include <utility>

namespace GHULBUS_VULKAN_NAMESPACE {

namespace {

void* static_allocate(void* pUserData, std::size_t size, std::size_t alignment, VkSystemAllocationScope allocationScope) {
    return reinterpret_cast<HostMemoryAllocator_GlobalNew*>(pUserData)->allocate(size, alignment, allocationScope);
}

void* static_reallocate(void* pUserData, void* pOriginal, std::size_t size, std::size_t alignment,
                        VkSystemAllocationScope allocationScope)
{
    return reinterpret_cast<HostMemoryAllocator_GlobalNew*>(pUserData)->reallocate(pOriginal, size,
                                                                                   alignment, allocationScope);
}

void static_free(void* pUserData, void* pMemory) {
    return reinterpret_cast<HostMemoryAllocator_GlobalNew*>(pUserData)->free(pMemory);

}

void static_internalAllocationNotification(void* pUserData, std::size_t size, VkInternalAllocationType allocationType,
                                           VkSystemAllocationScope allocationScope)
{
    return reinterpret_cast<HostMemoryAllocator_GlobalNew*>(pUserData)->internalAllocationNotification(size, allocationType,
                                                                                                       allocationScope);
}

void static_internalFreeNotification(void* pUserData, std::size_t size, VkInternalAllocationType allocationType,
                                     VkSystemAllocationScope allocationScope)
{
    return reinterpret_cast<HostMemoryAllocator_GlobalNew*>(pUserData)->internalFreeNotification(size, allocationType,
                                                                                                 allocationScope);
}

}

HostMemoryAllocator_GlobalNew::HostMemoryAllocator_GlobalNew() = default;
HostMemoryAllocator_GlobalNew::~HostMemoryAllocator_GlobalNew() = default;

VkAllocationCallbacks HostMemoryAllocator_GlobalNew::getVkAllocationCallbacks() {
    VkAllocationCallbacks ret;
    ret.pUserData = this;
    ret.pfnAllocation = static_allocate;
    ret.pfnReallocation = static_reallocate;
    ret.pfnFree = static_free;
    ret.pfnInternalAllocation = static_internalAllocationNotification;
    ret.pfnInternalFree = static_internalFreeNotification;
    return ret;
}

void* HostMemoryAllocator_GlobalNew::allocate(std::size_t size, std::size_t alignment,
                                              VkSystemAllocationScope allocationScope)
{
    GHULBUS_PRECONDITION(size > 0);
    void* mem = ::operator new(size, std::align_val_t{ alignment }, std::nothrow);
    if (mem) {
        GHULBUS_ASSERT_DBG(!m_activeAllocations.contains(mem));
        m_activeAllocations[mem] = AllocationInfo{ .size = size, .alignment = alignment, .scope = allocationScope };
    }
    return mem;
}

void* HostMemoryAllocator_GlobalNew::reallocate(void* pOriginal, std::size_t size, std::size_t alignment,
                                                VkSystemAllocationScope allocationScope)
{
    if (!pOriginal) {
        return allocate(size, alignment, allocationScope);
    } else if (size == 0) {
        this->free(pOriginal);
        return nullptr;
    } else {
        GHULBUS_PRECONDITION(m_activeAllocations.contains(pOriginal));
        AllocationInfo const& alloc_info = m_activeAllocations[pOriginal];
        void* mem = allocate(size, std::max(alloc_info.alignment, alignment), allocationScope);
        if (mem) {
            std::memcpy(mem, pOriginal, std::min(size, alloc_info.size));
            this->free(pOriginal);
        }
        return mem;
    }
    
}

void HostMemoryAllocator_GlobalNew::free(void* pMemory) {
    auto const alloc_info_it = m_activeAllocations.find(pMemory);
    GHULBUS_PRECONDITION(alloc_info_it != m_activeAllocations.end());
    AllocationInfo const& alloc_info = alloc_info_it->second;
    ::operator delete(pMemory, alloc_info.size, std::align_val_t{ alloc_info.alignment });
    m_activeAllocations.erase(alloc_info_it);
}

void HostMemoryAllocator_GlobalNew::internalAllocationNotification(std::size_t size, VkInternalAllocationType allocationType,
                                                                   VkSystemAllocationScope allocationScope)
{
    GHULBUS_UNUSED_VARIABLE(size);
    GHULBUS_UNUSED_VARIABLE(allocationType);
    GHULBUS_UNUSED_VARIABLE(allocationScope);
}
void HostMemoryAllocator_GlobalNew::internalFreeNotification(std::size_t size, VkInternalAllocationType allocationType,
                                                             VkSystemAllocationScope allocationScope)
{
    GHULBUS_UNUSED_VARIABLE(size);
    GHULBUS_UNUSED_VARIABLE(allocationType);
    GHULBUS_UNUSED_VARIABLE(allocationScope);
}

}
