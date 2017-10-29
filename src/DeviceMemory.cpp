#include <gbVk/DeviceMemory.hpp>

#include <gbVk/Exceptions.hpp>

#include <utility>

namespace GHULBUS_VULKAN_NAMESPACE
{
DeviceMemory::MappedMemory::MappedMemory(VkDevice device, VkDeviceMemory memory, void* mapped_memory)
    :m_mappedMemory(mapped_memory), m_device(device), m_memory(memory)
{
}

DeviceMemory::MappedMemory::~MappedMemory()
{
    if(m_mappedMemory) {
        vkUnmapMemory(m_device, m_memory);
    }
}

DeviceMemory::MappedMemory::MappedMemory(MappedMemory&& rhs)
    :m_mappedMemory(rhs.m_mappedMemory), m_device(rhs.m_device), m_memory(rhs.m_memory)
{
    rhs.m_mappedMemory = nullptr;
    rhs.m_device = nullptr;
    rhs.m_memory = nullptr;
}

DeviceMemory::MappedMemory& DeviceMemory::MappedMemory::operator=(MappedMemory&& rhs)
{
    if(m_mappedMemory) { vkUnmapMemory(m_device, m_memory); }
    m_mappedMemory = nullptr;
    m_device = nullptr;
    m_memory = nullptr;
    std::swap(m_mappedMemory, rhs.m_mappedMemory);
    std::swap(m_device, rhs.m_device);
    std::swap(m_memory, rhs.m_memory);
    return *this;
}

DeviceMemory::MappedMemory::operator std::byte*()
{
    return reinterpret_cast<std::byte*>(m_mappedMemory);
}

std::byte& DeviceMemory::MappedMemory::operator[](std::size_t index)
{
    return reinterpret_cast<std::byte*>(m_mappedMemory)[index];
}

DeviceMemory::MappedMemory::operator std::byte const*() const
{
    return reinterpret_cast<std::byte const*>(m_mappedMemory);
}

std::byte DeviceMemory::MappedMemory::operator[](std::size_t index) const
{
    return reinterpret_cast<std::byte const*>(m_mappedMemory)[index];
}

void DeviceMemory::MappedMemory::flush()
{
    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = m_memory;
    range.offset = 0;
    range.size = VK_WHOLE_SIZE;
    VkResult res = vkFlushMappedMemoryRanges(m_device, 1, &range);
    checkVulkanError(res, "Error in vkFlushMappedMemoryRanges.");
}

void DeviceMemory::MappedMemory::invalidate()
{
    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = m_memory;
    range.offset = 0;
    range.size = VK_WHOLE_SIZE;
    VkResult res = vkInvalidateMappedMemoryRanges(m_device, 1, &range);
    checkVulkanError(res, "Error in vkInvalidateMappedMemoryRanges.");
}

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

DeviceMemory::MappedMemory DeviceMemory::map()
{
    return map(0, VK_WHOLE_SIZE);
}

DeviceMemory::MappedMemory DeviceMemory::map(VkDeviceSize offset, VkDeviceSize size)
{
    void* ret;
    VkResult res = vkMapMemory(m_device, m_memory, offset, size, 0, &ret);
    checkVulkanError(res, "Erro in vkMapMemory.");
    return MappedMemory(m_device, m_memory, ret);
}
}
