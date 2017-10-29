#include <gbVk/Device.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/PhysicalDevice.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Device::Device(VkPhysicalDevice physical_device, VkDevice logical_device)
    :m_device(logical_device), m_physicalDevice(physical_device)
{
}

Device::~Device()
{
    if(m_device) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
    }
}

Device::Device(Device&& rhs)
    :m_device(rhs.m_device), m_physicalDevice(rhs.m_physicalDevice)
{
    rhs.m_device = nullptr;
    rhs.m_physicalDevice = nullptr;
}

DeviceMemory Device::allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags)
{
    auto const memory_type_index = PhysicalDevice(m_physicalDevice).findMemoryTypeIndex(flags);
    if(!memory_type_index) {
        GHULBUS_THROW(Exceptions::ProtocolViolation(), "No matching memory type available.");
    }
    VkMemoryAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.allocationSize = requested_size;
    alloc_info.memoryTypeIndex = *memory_type_index;

    VkDeviceMemory mem;
    VkResult res = vkAllocateMemory(m_device, &alloc_info, nullptr, &mem);
    checkVulkanError(res, "Error in vkAllocateMemory.");
    return DeviceMemory(m_device, mem);
}

PhysicalDevice Device::getPhysicalDevice()
{
    return PhysicalDevice(m_physicalDevice);
}
}
