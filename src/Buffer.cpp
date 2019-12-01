#include <gbVk/Buffer.hpp>

#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Exceptions.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Buffer::Buffer(VkDevice logical_device, VkBuffer buffer)
    :m_buffer(buffer), m_device(logical_device)
{
}

Buffer::~Buffer()
{
    if(m_buffer) { vkDestroyBuffer(m_device, m_buffer, nullptr); }
}

Buffer::Buffer(Buffer&& rhs)
    :m_buffer(rhs.m_buffer), m_device(rhs.m_device)
{
    rhs.m_buffer = nullptr;
    rhs.m_device = nullptr;
}

VkBuffer Buffer::getVkBuffer()
{
    return m_buffer;
}

VkMemoryRequirements Buffer::getMemoryRequirements()
{
    VkMemoryRequirements ret;
    vkGetBufferMemoryRequirements(m_device, m_buffer, &ret);
    return ret;
}

void Buffer::bindBufferMemory(DeviceMemory& memory)
{
    bindBufferMemory(memory, 0);
}

void Buffer::bindBufferMemory(DeviceMemory& memory, VkDeviceSize offset)
{
    VkResult const res = vkBindBufferMemory(m_device, m_buffer, memory.getVkDeviceMemory(), offset);
    checkVulkanError(res, "Error in vkBindBufferMemory.");
}
}
