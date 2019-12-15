#include <gbVk/SubmitStaging.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Semaphore.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
void SubmitStaging::addWaitingSemaphore(Semaphore& semaphore, VkPipelineStageFlags stage_to_wait)
{
    m_waitingSemaphores.push_back(semaphore.getVkSemaphore());
    m_waitingSemaphoreStageFlags.push_back(stage_to_wait);
}

void SubmitStaging::addSignalingSemaphore(Semaphore& semaphore)
{
    m_signallingSemaphores.push_back(semaphore.getVkSemaphore());
}

void SubmitStaging::addCommandBuffer(CommandBuffer& command_buffer)
{
    m_commandBuffers.push_back(command_buffer.getVkCommandBuffer());
}

void SubmitStaging::addCommandBuffers(CommandBuffers& command_buffers)
{
    VkCommandBuffer* p = command_buffers.getVkCommandBuffers();
    m_commandBuffers.insert(m_commandBuffers.end(), p, p + command_buffers.size());
}

void SubmitStaging::addCleanupCallback(Callback cb)
{
    m_callbacks.emplace_back(std::move(cb));
}

void SubmitStaging::performCleanup()
{
    for (auto& cb : m_callbacks) {
        Callback c = std::move(cb);
        c();
    }
    m_callbacks.clear();
}

VkSubmitInfo SubmitStaging::getVkSubmitInfo() const
{
    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    std::vector<VkSemaphore> const& ws = m_waitingSemaphores;
    submit_info.waitSemaphoreCount = static_cast<uint32_t>(ws.size());
    submit_info.pWaitSemaphores = (ws.empty()) ? nullptr : ws.data();
    std::vector<VkPipelineStageFlags> const& wsf = m_waitingSemaphoreStageFlags;
    GHULBUS_ASSERT(wsf.size() == ws.size());
    submit_info.pWaitDstStageMask = (ws.empty()) ? nullptr : wsf.data();
    std::vector<VkCommandBuffer> const& cb = m_commandBuffers;
    submit_info.commandBufferCount = static_cast<uint32_t>(cb.size());
    submit_info.pCommandBuffers = (cb.empty()) ? nullptr : cb.data();
    std::vector<VkSemaphore> const& ss = m_signallingSemaphores;
    submit_info.signalSemaphoreCount = static_cast<uint32_t>(ss.size());
    submit_info.pSignalSemaphores = (ss.empty()) ? nullptr : ss.data();
    return submit_info;
}
}
