#include <gbVk/CommandBuffer.hpp>

#include <gbVk/Exceptions.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

CommandBuffer::CommandBuffer(VkCommandBuffer command_buffer)
    :m_commandBuffer(command_buffer)
{}

CommandBuffer::~CommandBuffer()
{
    // @todo where to free?
}

CommandBuffer::CommandBuffer(CommandBuffer&& rhs)
    :m_commandBuffer(rhs.m_commandBuffer)
{
    rhs.m_commandBuffer = nullptr;
}

VkCommandBuffer CommandBuffer::getVkCommandBuffer()
{
    return m_commandBuffer;
}

void CommandBuffer::begin()
{
    begin(0);
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = nullptr;
    VkResult res = vkBeginCommandBuffer(m_commandBuffer, &begin_info);
    checkVulkanError(res, "Error in vkBeginCommandBuffer.");
}

void CommandBuffer::end()
{
    VkResult res = vkEndCommandBuffer(m_commandBuffer);
    checkVulkanError(res, "Error in vkEndCommandBuffer.");
}

void CommandBuffer::reset()
{
    reset(0);
}

void CommandBuffer::reset(VkCommandBufferResetFlags flags)
{
    VkResult res = vkResetCommandBuffer(m_commandBuffer, flags);
    checkVulkanError(res, "Error in vkResetCommandBuffer.");
}

void CommandBuffer::submit(VkQueue queue)
{
    //@todo this should probably be a member of a queue type instead
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_commandBuffer;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    VkFence fence = VK_NULL_HANDLE;
    VkResult res = vkQueueSubmit(queue, 1, &submit_info, fence);
    checkVulkanError(res, "Error in vkQueueSubmit.");
    res = vkQueueWaitIdle(queue);
    checkVulkanError(res, "Error in vkQueueWaitIdle.");
}
}
