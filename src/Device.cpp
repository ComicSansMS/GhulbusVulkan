#include <gbVk/Device.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Device::Device(VkDevice logical_device)
    :m_device(logical_device)
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
    :m_device(rhs.m_device)
{
    rhs.m_device = nullptr;
}
}
