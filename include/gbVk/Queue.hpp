#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_QUEUE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_QUEUE_HPP

/** @file
*
* @brief Queue.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <gbVk/SubmitStaging.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <functional>

namespace GHULBUS_VULKAN_NAMESPACE
{
class CommandBuffer;
class CommandBuffers;
class Fence;

class Queue {
private:
    VkQueue m_queue;
    std::vector<SubmitStaging> m_staged;
    std::vector<VkSubmitInfo> m_cachedSubmitInfo;       /// this is only kept around so we don't have to reallocate the vector on each submitAllStaged()
public:
    Queue(VkQueue queue);

    void submit(CommandBuffer& command_buffer);

    void submit(CommandBuffers& command_buffers);

    void submit(CommandBuffers& command_buffers, Fence& fence);

    void stageSubmission(SubmitStaging staging);

    void submitAllStaged();

    void submitAllStaged(Fence& fence);

    void clearAllStaged();

    void waitIdle();

    VkQueue getVkQueue();
};
}
#endif
