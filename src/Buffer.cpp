#include <gbVk/Buffer.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

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

void Buffer::transitionRelease(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                               VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
                               uint32_t dst_queue_family)
{
    transitionRelease(command_buffer, src_stage, dst_stage, src_access_mask, dst_queue_family, 0, VK_WHOLE_SIZE);
}

void Buffer::transitionRelease(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                               VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
                               uint32_t dst_queue_family, VkDeviceSize offset, VkDeviceSize size)
{
    VkBufferMemoryBarrier buffer_barr;
    buffer_barr.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buffer_barr.pNext = nullptr;
    buffer_barr.srcAccessMask = src_access_mask;
    buffer_barr.dstAccessMask = 0;
    buffer_barr.srcQueueFamilyIndex = command_buffer.getQueueFamilyIndex();
    buffer_barr.dstQueueFamilyIndex = dst_queue_family;
    buffer_barr.buffer = m_buffer;
    buffer_barr.offset = offset;
    buffer_barr.size = size;
    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(), src_stage, dst_stage, 0,
                         0, nullptr, 1, &buffer_barr, 0, nullptr);
}

void Buffer::transitionAcquire(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                               VkPipelineStageFlags dst_stage, VkAccessFlags dst_access_mask,
                               uint32_t src_queue_family)
{
    transitionAcquire(command_buffer, src_stage, dst_stage, dst_access_mask, src_queue_family, 0, VK_WHOLE_SIZE);
}

void Buffer::transitionAcquire(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                               VkPipelineStageFlags dst_stage, VkAccessFlags dst_access_mask,
                               uint32_t src_queue_family, VkDeviceSize offset, VkDeviceSize size)
{
    VkBufferMemoryBarrier buffer_barr;
    buffer_barr.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buffer_barr.pNext = nullptr;
    buffer_barr.srcAccessMask = 0;
    buffer_barr.dstAccessMask = dst_access_mask;
    buffer_barr.srcQueueFamilyIndex = src_queue_family;
    buffer_barr.dstQueueFamilyIndex = command_buffer.getQueueFamilyIndex();
    buffer_barr.buffer = m_buffer;
    buffer_barr.offset = offset;
    buffer_barr.size = size;
    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(), src_stage, dst_stage, 0,
                         0, nullptr, 1, &buffer_barr, 0, nullptr);
}
}
