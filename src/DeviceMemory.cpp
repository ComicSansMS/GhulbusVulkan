#include <gbVk/DeviceMemory.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DeviceMemory::DeviceMemory(VkDevice device, VkDeviceMemory memory)
    :m_memory(memory), m_device(device)
{
}

DeviceMemory::~DeviceMemory()
{
    if(m_memory) {
        vkFreeMemory(m_device, m_memory, nullptr);
    }
}

DeviceMemory::DeviceMemory(DeviceMemory&& rhs)
    :m_memory(rhs.m_memory), m_device(rhs.m_device)
{
    rhs.m_memory = nullptr;
    rhs.m_device = nullptr;
}

VkDeviceSize DeviceMemory::getCommitment()
{
    VkDeviceSize ret;
    vkGetDeviceMemoryCommitment(m_device, m_memory, &ret);
    return ret;
}
}
