#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_QUEUE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_QUEUE_HPP

/** @file
*
* @brief Queue.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>


namespace GHULBUS_VULKAN_NAMESPACE
{
class CommandBuffer;
class CommandBuffers;
class Fence;

class Queue {
private:
    VkQueue m_queue;
public:
    Queue(VkQueue queue);

    void submit(CommandBuffer& command_buffer);

    void submit(CommandBuffers& command_buffers);

    void submit(CommandBuffers& command_buffers, Fence& fence);

    void waitIdle();

    VkQueue getVkQueue();
};
}
#endif
