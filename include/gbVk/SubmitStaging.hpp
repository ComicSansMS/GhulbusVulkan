#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SUBMIT_STAGING_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SUBMIT_STAGING_HPP

/** @file
*
* @brief Submit Staging.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <gbBase/AnyInvocable.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class CommandBuffer;
class CommandBuffers;
class Semaphore;

class [[nodiscard]] SubmitStaging {
public:
    using Callback = Ghulbus::AnyInvocable<void()>;
private:
    std::vector<VkCommandBufferSubmitInfo> m_commandBufferInfos;
    std::vector<VkSemaphoreSubmitInfo> m_waitingSemaphoreInfos;
    std::vector<VkSemaphoreSubmitInfo> m_signallingSemaphoreInfos;
    std::vector<Callback> m_callbacks;
public:
    SubmitStaging() = default;

    void addWaitingSemaphore(Semaphore& semaphore, VkPipelineStageFlags2 stage_to_wait);

    void addSignalingSemaphore(Semaphore& semaphore, VkPipelineStageFlags2 stage_to_wait);

    void addCommandBuffer(CommandBuffer& command_buffer);

    void addCommandBuffers(CommandBuffers& command_buffers);

    void addCleanupCallback(Callback cb);

    void performCleanup();

    VkSubmitInfo2 getVkSubmitInfo() const;

    template<typename... Args>
    void adoptResources(Args&&... args) {
        addCleanupCallback([... args = std::forward<Args>(args)]() {});
    }
};
}
#endif
