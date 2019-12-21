#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_BUFFERS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_BUFFERS_HPP

/** @file
*
* @brief Command Buffers.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <gbVk/CommandBuffer.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

class CommandBuffers
{
private:
    std::vector<VkCommandBuffer> m_commandBuffers;
    VkDevice m_device;
    VkCommandPool m_commandPool;
    uint32_t m_queueFamilyIndex;
    std::vector<CommandBuffer> m_commandBufferObjects;
public:
    CommandBuffers();

    CommandBuffers(VkDevice logical_device, VkCommandPool command_pool,
                   std::vector<VkCommandBuffer> command_buffers, uint32_t queue_family_index);

    ~CommandBuffers();

    CommandBuffers(CommandBuffers const&) = delete;
    CommandBuffers& operator=(CommandBuffers const&) = delete;

    CommandBuffers(CommandBuffers&& rhs);
    CommandBuffers& operator=(CommandBuffers&& rhs);

    uint32_t size() const;

    uint32_t getQueueFamilyIndex() const;

    CommandBuffer& getCommandBuffer(uint32_t index);

    VkCommandBuffer* getVkCommandBuffers();
};
}
#endif
