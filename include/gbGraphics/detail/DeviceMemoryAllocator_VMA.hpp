#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_DEVICE_MEMORY_ALLOCATOR_VMA_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_DEVICE_MEMORY_ALLOCATOR_VMA_HPP

/** @file
*
* @brief Device Memory Allocator based on GpuOpen Vulkan Memory Allocator.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>
#include <gbVk/DeviceMemoryAllocator.hpp>

#include <vk_mem_alloc.h>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
using GhulbusVulkan::MemoryUsage;
namespace detail
{
class DeviceMemoryAllocator_VMA : public GhulbusVulkan::DeviceMemoryAllocator {
private:
    class HandleModel : public DeviceMemoryAllocator::HandleConcept {
    private:
        VmaAllocation m_allocation;
        VmaAllocator m_allocator;
        VmaAllocationInfo m_allocationInfo;
    public:
        HandleModel(VmaAllocator allocator, VmaAllocation allocation, VmaAllocationInfo const& allocation_info);
        ~HandleModel() override;
        HandleModel(HandleModel const&) = delete;
        HandleModel& operator=(HandleModel const&) = delete;

        VkDeviceMemory getVkDeviceMemory() const override;
        VkDeviceSize getOffset() const override;
        VkDeviceSize getSize() const override;
        void* mapMemory(VkDeviceSize offset, VkDeviceSize size) override;
        void unmapMemory(void* mapped_memory) override;
        void flush(VkDeviceSize offset, VkDeviceSize size) override;
        void invalidate(VkDeviceSize offset, VkDeviceSize size) override;
        void bindBuffer(VkBuffer buffer) override;
        void bindImage(VkImage image) override;
    };
private:
    VmaAllocator m_allocator;
public:
    DeviceMemoryAllocator_VMA(GhulbusVulkan::Instance& instance, GhulbusVulkan::Device& device);

    ~DeviceMemoryAllocator_VMA() override;

    DeviceMemoryAllocator_VMA(DeviceMemoryAllocator_VMA const&) = delete;
    DeviceMemoryAllocator_VMA& operator=(DeviceMemoryAllocator_VMA const&) = delete;

    DeviceMemoryAllocator_VMA(DeviceMemoryAllocator_VMA&& rhs);
    DeviceMemoryAllocator_VMA& operator=(DeviceMemoryAllocator_VMA&& rhs);

    DeviceMemory allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags) override;
    DeviceMemory allocateMemory(VkMemoryRequirements const& requirements,
                                VkMemoryPropertyFlags required_flags) override;

    DeviceMemory allocateMemoryForBuffer(GhulbusVulkan::Buffer& buffer, MemoryUsage usage) override;
    DeviceMemory allocateMemoryForBuffer(GhulbusVulkan::Buffer& buffer,
                                         VkMemoryPropertyFlags required_flags) override;

    DeviceMemory allocateMemoryForImage(GhulbusVulkan::Image& image, MemoryUsage usage) override;
    DeviceMemory allocateMemoryForImage(GhulbusVulkan::Image& image,
                                        VkMemoryPropertyFlags required_flags) override;
private:
    static VmaMemoryUsage translateUsage(MemoryUsage usage);
};
}
}
#endif
