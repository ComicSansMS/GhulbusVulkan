#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_ALLOCATOR_TRIVIAL_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_ALLOCATOR_TRIVIAL_HPP

/** @file
*
* @brief Device Memory Allocator Trivial Implementation.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <gbVk/DeviceMemoryAllocator.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DeviceMemoryAllocator_Trivial final : public DeviceMemoryAllocator {
private:
    class HandleModel final : public DeviceMemoryAllocator::HandleConcept {
    private:
        VkDeviceMemory m_memory;
        VkDevice m_device;
        VkDeviceSize m_size;
    public:
        HandleModel(VkDevice device, VkDeviceMemory memory, VkDeviceSize size);
        ~HandleModel() override;
        VkDeviceMemory getVkDeviceMemory() const override;
        VkDeviceSize getOffset() const override;
        VkDeviceSize getSize() const override;
        void* mapMemory(VkDeviceSize offset, VkDeviceSize size) override;
        void unmapMemory(void* mapped_memory) override;
        void flush(VkDeviceSize offset, VkDeviceSize size) override;
        void invalidate(VkDeviceSize offset, VkDeviceSize size) override;
    };
private:
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
public:
    DeviceMemoryAllocator_Trivial(VkDevice logical_device, VkPhysicalDevice physical_device);

    ~DeviceMemoryAllocator_Trivial() override;

    DeviceMemoryAllocator_Trivial(DeviceMemoryAllocator_Trivial const&) = delete;
    DeviceMemoryAllocator_Trivial& operator=(DeviceMemoryAllocator_Trivial const&) = delete;

    DeviceMemoryAllocator_Trivial(DeviceMemoryAllocator_Trivial&& rhs);
    DeviceMemoryAllocator_Trivial& operator=(DeviceMemoryAllocator_Trivial&& rhs) = delete;

    DeviceMemory allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags) override;
    DeviceMemory allocateMemory(VkMemoryRequirements const& requirements,
                                VkMemoryPropertyFlags required_flags) override;

    DeviceMemory allocateMemoryForImage(Image& image, MemoryUsage usage) override;
    DeviceMemory allocateMemoryForImage(Image& image,
                                        VkMemoryPropertyFlags required_flags) override;

private:
    static VkMemoryPropertyFlags translateUsage(MemoryUsage usage);
};
}
#endif
