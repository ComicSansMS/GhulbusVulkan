#include <gbGraphics/detail/DeviceMemoryAllocator.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>


namespace GHULBUS_GRAPHICS_NAMESPACE::detail
{
DeviceMemoryAllocator::DeviceMemoryAllocator(GhulbusVulkan::Instance& instance, GhulbusVulkan::Device& device)
    :m_allocator(nullptr)
{
    VmaAllocatorCreateInfo create_info;
    create_info.flags = 0;
    create_info.physicalDevice = device.getPhysicalDevice().getVkPhysicalDevice();
    create_info.device = device.getVkDevice();
    create_info.preferredLargeHeapBlockSize = 0;
    create_info.pAllocationCallbacks = nullptr;
    create_info.pDeviceMemoryCallbacks = nullptr;
    create_info.frameInUseCount = 0;
    create_info.pHeapSizeLimit = nullptr;
    create_info.pVulkanFunctions = nullptr;
    create_info.pRecordSettings = nullptr;
    create_info.instance = instance.getVkInstance();
    create_info.vulkanApiVersion = instance.getVulkanApiVersion();
    VkResult const res = vmaCreateAllocator(&create_info, &m_allocator);
    GhulbusVulkan::checkVulkanError(res, "Error in vmaCreateAllocator.");
}

DeviceMemoryAllocator::~DeviceMemoryAllocator()
{
    if(m_allocator) {
        vmaDestroyAllocator(m_allocator);
    }
}

DeviceMemoryAllocator::DeviceMemoryAllocator(DeviceMemoryAllocator&& rhs)
    :m_allocator(rhs.m_allocator)
{
    rhs.m_allocator = nullptr;
}

DeviceMemoryAllocator& DeviceMemoryAllocator::operator=(DeviceMemoryAllocator&& rhs)
{
    if (this != &rhs) {
        if (m_allocator) { vmaDestroyAllocator(m_allocator); }
        m_allocator = rhs.m_allocator;
        rhs.m_allocator = nullptr;
    }
    return *this;
}
}
