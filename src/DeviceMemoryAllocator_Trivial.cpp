#include <gbVk/DeviceMemoryAllocator_Trivial.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/PhysicalDevice.hpp>

#include <gbBase/UnusedVariable.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DeviceMemoryAllocator_Trivial::HandleModel::HandleModel(VkDevice device, VkDeviceMemory memory, VkDeviceSize size)
    :m_memory(memory), m_device(device), m_size(size)
{}

DeviceMemoryAllocator_Trivial::HandleModel::~HandleModel()
{
    if(m_memory) {
        vkFreeMemory(m_device, m_memory, nullptr);
    }
}

VkDeviceMemory DeviceMemoryAllocator_Trivial::HandleModel::getVkDeviceMemory() const
{
    return m_memory;
}

VkDeviceSize DeviceMemoryAllocator_Trivial::HandleModel::getOffset() const
{
    return 0;
}

VkDeviceSize DeviceMemoryAllocator_Trivial::HandleModel::getSize() const
{
    return m_size;
}

void* DeviceMemoryAllocator_Trivial::HandleModel::mapMemory(VkDeviceSize offset, VkDeviceSize size)
{
    void* ret;
    VkResult res = vkMapMemory(m_device, m_memory, offset, size, 0, &ret);
    checkVulkanError(res, "Erro in vkMapMemory.");
    return ret;
}

void DeviceMemoryAllocator_Trivial::HandleModel::unmapMemory(void* mapped_memory)
{
    GHULBUS_UNUSED_VARIABLE(mapped_memory);
    vkUnmapMemory(m_device, m_memory);
}

void DeviceMemoryAllocator_Trivial::HandleModel::flush(VkDeviceSize offset, VkDeviceSize size)
{
    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = m_memory;
    range.offset = offset;
    range.size = size;
    VkResult res = vkFlushMappedMemoryRanges(m_device, 1, &range);
    checkVulkanError(res, "Error in vkFlushMappedMemoryRanges.");
}

void DeviceMemoryAllocator_Trivial::HandleModel::invalidate(VkDeviceSize offset, VkDeviceSize size)
{
    VkMappedMemoryRange range;
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.pNext = nullptr;
    range.memory = m_memory;
    range.offset = offset;
    range.size = size;
    VkResult res = vkInvalidateMappedMemoryRanges(m_device, 1, &range);
    checkVulkanError(res, "Error in vkInvalidateMappedMemoryRanges.");
}

DeviceMemoryAllocator_Trivial::DeviceMemoryAllocator_Trivial(VkDevice logical_device, VkPhysicalDevice physical_device)
    : m_device(logical_device), m_physicalDevice(physical_device)
{}

DeviceMemoryAllocator_Trivial::~DeviceMemoryAllocator_Trivial() = default;

DeviceMemoryAllocator_Trivial::DeviceMemoryAllocator_Trivial(DeviceMemoryAllocator_Trivial&& rhs)
    :m_device(rhs.m_device), m_physicalDevice(rhs.m_physicalDevice)
{
    rhs.m_device = nullptr;
    rhs.m_physicalDevice = nullptr;
}

auto DeviceMemoryAllocator_Trivial::allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags) -> DeviceMemory
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
    return DeviceMemory(std::make_unique<HandleModel>(m_device, mem, requested_size));
}

auto DeviceMemoryAllocator_Trivial::allocateMemory(VkMemoryRequirements const& requirements,
                                                   VkMemoryPropertyFlags required_flags) -> DeviceMemory
{
    auto const memory_type_index = PhysicalDevice(m_physicalDevice).findMemoryTypeIndex(required_flags, requirements);
    if(!memory_type_index) {
        GHULBUS_THROW(Exceptions::ProtocolViolation(), "No matching memory type available.");
    }
    VkMemoryAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = *memory_type_index;

    VkDeviceMemory mem;
    VkResult res = vkAllocateMemory(m_device, &alloc_info, nullptr, &mem);
    checkVulkanError(res, "Error in vkAllocateMemory.");
    return DeviceMemory(std::make_unique<HandleModel>(m_device, mem, requirements.size));
}

auto DeviceMemoryAllocator_Trivial::allocateMemoryForImage(Image& image, MemoryUsage usage) -> DeviceMemory
{
    ///@todo
    GHULBUS_THROW(Exceptions::NotImplemented{}, "Not yet implemented.");
}

auto DeviceMemoryAllocator_Trivial::allocateMemoryForImage(Image& image,
                                                           VkMemoryRequirements const& requirements,
                                                           VkMemoryPropertyFlags required_flags) -> DeviceMemory
{
    GHULBUS_UNUSED_VARIABLE(image);
    return allocateMemory(requirements, required_flags);
}
}
