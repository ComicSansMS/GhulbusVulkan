#include <gbGraphics/detail/DeviceMemoryAllocator_VMA.hpp>

#include <gbGraphics/Exceptions.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>

#include <gbBase/UnusedVariable.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE::detail
{

DeviceMemoryAllocator_VMA::HandleModel::HandleModel(VmaAllocator allocator, VmaAllocation allocation,
    VmaAllocationInfo const& allocation_info)
    :m_allocation(allocation), m_allocator(allocator), m_allocationInfo(allocation_info)
{
}

DeviceMemoryAllocator_VMA::HandleModel::~HandleModel()
{
    vmaFreeMemory(m_allocator, m_allocation);
}

VkDeviceMemory DeviceMemoryAllocator_VMA::HandleModel::getVkDeviceMemory() const
{
    /// @attention this will change if we ever allow defragmentation of VMA
    return m_allocationInfo.deviceMemory;
}

VkDeviceSize DeviceMemoryAllocator_VMA::HandleModel::getOffset() const
{
    /// @attention this will change if we ever allow defragmentation of VMA
    return m_allocationInfo.offset;
}

VkDeviceSize DeviceMemoryAllocator_VMA::HandleModel::getSize() const
{
    return m_allocationInfo.size;
}

void* DeviceMemoryAllocator_VMA::HandleModel::mapMemory(VkDeviceSize offset, VkDeviceSize size)
{
    GHULBUS_UNUSED_VARIABLE(size);
    void* mapped;
    VkResult const res = vmaMapMemory(m_allocator, m_allocation, &mapped);
    GhulbusVulkan::checkVulkanError(res, "Error in vmaMapMemory.");
    return static_cast<std::byte*>(mapped) + offset;
}

void DeviceMemoryAllocator_VMA::HandleModel::unmapMemory(void* mapped_memory)
{
    GHULBUS_UNUSED_VARIABLE(mapped_memory);
    vmaUnmapMemory(m_allocator, m_allocation);
}

void DeviceMemoryAllocator_VMA::HandleModel::flush(VkDeviceSize offset, VkDeviceSize size)
{
    vmaFlushAllocation(m_allocator, m_allocation, offset, size);
}

void DeviceMemoryAllocator_VMA::HandleModel::invalidate(VkDeviceSize offset, VkDeviceSize size)
{
    vmaInvalidateAllocation(m_allocator, m_allocation, offset, size);
}

DeviceMemoryAllocator_VMA::DeviceMemoryAllocator_VMA(GhulbusVulkan::Instance& instance, GhulbusVulkan::Device& device)
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

DeviceMemoryAllocator_VMA::~DeviceMemoryAllocator_VMA()
{
    if(m_allocator) {
        vmaDestroyAllocator(m_allocator);
    }
}

DeviceMemoryAllocator_VMA::DeviceMemoryAllocator_VMA(DeviceMemoryAllocator_VMA&& rhs)
    :m_allocator(rhs.m_allocator)
{
    rhs.m_allocator = nullptr;
}

DeviceMemoryAllocator_VMA& DeviceMemoryAllocator_VMA::operator=(DeviceMemoryAllocator_VMA&& rhs)
{
    if (this != &rhs) {
        if (m_allocator) { vmaDestroyAllocator(m_allocator); }
        m_allocator = rhs.m_allocator;
        rhs.m_allocator = nullptr;
    }
    return *this;
}

VmaMemoryUsage DeviceMemoryAllocator_VMA::translateUsage(GhulbusVulkan::MemoryUsage usage)
{
    using GhulbusVulkan::MemoryUsage;
    switch (usage) {
    case MemoryUsage::GpuOnly: return VMA_MEMORY_USAGE_GPU_ONLY;
    case MemoryUsage::CpuOnly: return VMA_MEMORY_USAGE_CPU_ONLY;
    case MemoryUsage::CpuToGpu: return VMA_MEMORY_USAGE_CPU_TO_GPU;
    case MemoryUsage::GpuToCpu: return VMA_MEMORY_USAGE_GPU_TO_CPU;
    default: GHULBUS_THROW(Exceptions::ProtocolViolation{}, "Invalid memory usage.");
    }
}

auto DeviceMemoryAllocator_VMA::allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags) -> DeviceMemory
{
    VkMemoryRequirements requirements;
    requirements.alignment = 0;
    requirements.size = requested_size;
    requirements.memoryTypeBits = 0;
    return allocateMemory(requirements, flags);
}

auto DeviceMemoryAllocator_VMA::allocateMemory(VkMemoryRequirements const& requirements,
                                               VkMemoryPropertyFlags required_flags)  -> DeviceMemory
{
    VmaAllocationCreateInfo create_info;
    create_info.flags = 0;
    create_info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    create_info.requiredFlags = required_flags;
    create_info.preferredFlags = 0;
    create_info.memoryTypeBits = requirements.memoryTypeBits;
    create_info.pool = VK_NULL_HANDLE;
    create_info.pUserData = this;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    VkResult const res =
        vmaAllocateMemory(m_allocator, &requirements, &create_info, &allocation, &allocation_info);
    GhulbusVulkan::checkVulkanError(res, "Error in vmaAllocateMemoryForImage.");
    return DeviceMemory(std::make_unique<HandleModel>(m_allocator, allocation, allocation_info));
}

auto DeviceMemoryAllocator_VMA::allocateMemoryForImage(GhulbusVulkan::Image& image, GhulbusVulkan::MemoryUsage usage) -> DeviceMemory
{
    VmaAllocationCreateInfo create_info;
    create_info.flags = 0;
    create_info.usage = translateUsage(usage);
    create_info.requiredFlags = 0;
    create_info.preferredFlags = 0;
    create_info.memoryTypeBits = 0;
    create_info.pool = VK_NULL_HANDLE;
    create_info.pUserData = this;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    VkResult const res =
        vmaAllocateMemoryForImage(m_allocator, image.getVkImage(), &create_info, &allocation, &allocation_info);
    GhulbusVulkan::checkVulkanError(res, "Error in vmaAllocateMemoryForImage.");
    return DeviceMemory(std::make_unique<HandleModel>(m_allocator, allocation, allocation_info));
}

auto DeviceMemoryAllocator_VMA::allocateMemoryForImage(GhulbusVulkan::Image& image,
                                                       VkMemoryPropertyFlags required_flags) -> DeviceMemory
{
    VkMemoryRequirements const requirements = image.getMemoryRequirements();
    VmaAllocationCreateInfo create_info;
    create_info.flags = 0;
    create_info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    create_info.requiredFlags = required_flags;
    create_info.preferredFlags = 0;
    create_info.memoryTypeBits = requirements.memoryTypeBits;
    create_info.pool = VK_NULL_HANDLE;
    create_info.pUserData = this;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
    VkResult const res =
        vmaAllocateMemoryForImage(m_allocator, image.getVkImage(), &create_info, &allocation, &allocation_info);
    GhulbusVulkan::checkVulkanError(res, "Error in vmaAllocateMemoryForImage.");
    return DeviceMemory(std::make_unique<HandleModel>(m_allocator, allocation, allocation_info));
}
}
