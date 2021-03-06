#include <gbVk/Queue.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/SubmitStaging.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
namespace {
void queue_submit(VkQueue queue, VkCommandBuffer* command_buffers, uint32_t n_command_buffers, VkFence fence)
{
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = nullptr;
    submit_info.pWaitDstStageMask = nullptr;
    submit_info.commandBufferCount = n_command_buffers;
    submit_info.pCommandBuffers = command_buffers;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = nullptr;
    VkResult const res = vkQueueSubmit(queue, 1, &submit_info, fence);
    checkVulkanError(res, "Error in vkQueueSubmit.");
}
}

Queue::Queue(VkQueue queue)
    :m_queue(queue)
{
}

void Queue::submit(CommandBuffer& command_buffer)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Executable);
    VkCommandBuffer cmd_buf = command_buffer.getVkCommandBuffer();
    queue_submit(m_queue, &cmd_buf, 1, VK_NULL_HANDLE);
}

void Queue::submit(CommandBuffers& command_buffers)
{
    queue_submit(m_queue, command_buffers.getVkCommandBuffers(), command_buffers.size(), VK_NULL_HANDLE);
}

void Queue::submit(CommandBuffers& command_buffers, Fence& fence)
{
    queue_submit(m_queue, command_buffers.getVkCommandBuffers(), command_buffers.size(), fence.getVkFence());
}

void Queue::stageSubmission(SubmitStaging staging)
{
    m_staged.emplace_back(std::move(staging));
    VkSubmitInfo const submit_info = staging.getVkSubmitInfo();
    VkResult const res = vkQueueSubmit(m_queue, 1, &submit_info, nullptr);
    checkVulkanError(res, "Error in vkQueueSubmit.");
}

void Queue::submitAllStaged()
{
    uint32_t n_stages = static_cast<uint32_t>(m_staged.size());
    m_cachedSubmitInfo.resize(n_stages);
    for (uint32_t i = 0; i < n_stages; ++i) {
        m_cachedSubmitInfo[i] = m_staged[i].getVkSubmitInfo();
    }
    VkResult const res = vkQueueSubmit(m_queue, n_stages, m_cachedSubmitInfo.data(), nullptr);
    checkVulkanError(res, "Error in vkQueueSubmit.");
}

void Queue::submitAllStaged(Fence& fence)
{
    uint32_t n_stages = static_cast<uint32_t>(m_staged.size());
    m_cachedSubmitInfo.resize(n_stages);
    for (uint32_t i = 0; i < n_stages; ++i) {
        m_cachedSubmitInfo[i] = m_staged[i].getVkSubmitInfo();
    }
    VkResult const res = vkQueueSubmit(m_queue, n_stages, m_cachedSubmitInfo.data(), fence.getVkFence());
    checkVulkanError(res, "Error in vkQueueSubmit.");
}

void Queue::clearAllStaged()
{
    for (auto& s : m_staged) { s.performCleanup(); }
    m_staged.clear();
    m_cachedSubmitInfo.clear();
}

void Queue::waitIdle()
{
    VkResult const res = vkQueueWaitIdle(m_queue);
    checkVulkanError(res, "Error in vkQueueWaitIdle.");
}

VkQueue Queue::getVkQueue()
{
    return m_queue;
}
}
