#include <gbVk/SubmitStaging.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Semaphore.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
void SubmitStaging::addWaitingSemaphore(Semaphore& semaphore, VkPipelineStageFlags2 stage_to_wait)
{
    VkSemaphoreSubmitInfo ssi;
    ssi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    ssi.pNext = nullptr;
    ssi.semaphore = semaphore.getVkSemaphore();
    ssi.value = 0;
    ssi.stageMask = stage_to_wait;
    ssi.deviceIndex = 0;
    m_waitingSemaphoreInfos.push_back(ssi);
}

void SubmitStaging::addSignalingSemaphore(Semaphore& semaphore, VkPipelineStageFlags2 stage_to_wait)
{
    VkSemaphoreSubmitInfo ssi;
    ssi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    ssi.pNext = nullptr;
    ssi.semaphore = semaphore.getVkSemaphore();
    ssi.value = 0;
    ssi.stageMask = stage_to_wait;
    ssi.deviceIndex = 0;
    m_signallingSemaphoreInfos.push_back(ssi);
}

void SubmitStaging::addCommandBuffer(CommandBuffer& command_buffer)
{
    VkCommandBufferSubmitInfo cbsi;
    cbsi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    cbsi.pNext = nullptr;
    cbsi.commandBuffer = command_buffer.getVkCommandBuffer();
    cbsi.deviceMask = 0;
    m_commandBufferInfos.push_back(cbsi);
}

void SubmitStaging::addCommandBuffers(CommandBuffers& command_buffers)
{
    for (uint32_t i = 0; i < command_buffers.size(); ++i) {
        addCommandBuffer(command_buffers.getCommandBuffer(i));
    }
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

VkSubmitInfo2 SubmitStaging::getVkSubmitInfo() const
{
    VkSubmitInfo2 submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.pNext = nullptr;
    submit_info.flags = 0;
    auto const& ws = m_waitingSemaphoreInfos;
    submit_info.waitSemaphoreInfoCount = static_cast<uint32_t>(m_waitingSemaphoreInfos.size());
    submit_info.pWaitSemaphoreInfos = (ws.empty()) ? nullptr : ws.data();
    auto const& cb = m_commandBufferInfos;
    submit_info.commandBufferInfoCount = static_cast<uint32_t>(cb.size());
    submit_info.pCommandBufferInfos = (cb.empty()) ? nullptr : cb.data();
    auto const& ss = m_signallingSemaphoreInfos;
    submit_info.signalSemaphoreInfoCount = static_cast<uint32_t>(ss.size());
    submit_info.pSignalSemaphoreInfos = (ss.empty()) ? nullptr : ss.data();
    return submit_info;
}
}
