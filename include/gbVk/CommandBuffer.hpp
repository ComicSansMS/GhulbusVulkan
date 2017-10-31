#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_BUFFER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_BUFFER_HPP

/** @file
*
* @brief Command Buffer.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

class CommandBuffer {
private:
    VkCommandBuffer m_commandBuffer;
public:
    explicit CommandBuffer(VkCommandBuffer command_buffer);
    ~CommandBuffer();

    CommandBuffer(CommandBuffer const&) = delete;
    CommandBuffer& operator=(CommandBuffer const&) = delete;

    CommandBuffer(CommandBuffer&& rhs);
    CommandBuffer& operator=(CommandBuffer&& rhs) = delete;

    VkCommandBuffer getVkCommandBuffer();

    void begin();
    void begin(VkCommandBufferUsageFlags flags);
    void end();

    void reset();
    void reset(VkCommandBufferResetFlags flags);

    void submit(VkQueue queue);
};
}
#endif
