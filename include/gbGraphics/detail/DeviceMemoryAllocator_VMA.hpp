#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_DEVICE_MEMORY_ALLOCATOR_VMA_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_DEVICE_MEMORY_ALLOCATOR_VMA_HPP

/** @file
*
* @brief Device Memory Allocator based on GpuOpen Vulkan Memory Allocator.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/DeviceMemoryAllocator.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <vk_mem_alloc.h>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
namespace detail
{
class DeviceMemoryAllocator_VMA : public ::GHULBUS_GRAPHICS_NAMESPACE::DeviceMemoryAllocator {
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

    Handle allocateMemoryForImage(GhulbusVulkan::Image& image, MemoryUsage usage) override;
    Handle allocateMemoryForImage(GhulbusVulkan::Image& image,
                                  VkMemoryRequirements const& requirements,
                                  VkMemoryPropertyFlags required_flags) override;
private:
    static VmaMemoryUsage translateUsage(::GHULBUS_GRAPHICS_NAMESPACE::MemoryUsage usage);
};
}
}
#endif
