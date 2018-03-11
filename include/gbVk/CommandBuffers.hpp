#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_BUFFERS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_BUFFERS_HPP

/** @file
*
* @brief Command Buffers.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class CommandBuffer;

class CommandBuffers
{
private:
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkDevice m_device;
    VkCommandPool m_commandPool;
public:
    CommandBuffers(VkDevice logical_device, VkCommandPool command_pool,
                   std::vector<VkCommandBuffer> command_buffers);

    ~CommandBuffers();

    CommandBuffers(CommandBuffers const&) = delete;
    CommandBuffers& operator=(CommandBuffers const&) = delete;

    CommandBuffers(CommandBuffers&& rhs);
    CommandBuffers& operator=(CommandBuffers&&) = delete;

    uint32_t size() const;

    CommandBuffer getCommandBuffer(uint32_t index);
};
}
#endif
