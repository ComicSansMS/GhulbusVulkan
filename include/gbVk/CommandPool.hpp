#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_POOL_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_COMMAND_POOL_HPP

/** @file
*
* @brief Command Pool.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class CommandBuffers;

class CommandPool {
private:
    VkCommandPool m_commandPool;
    VkDevice m_device;
    uint32_t m_queueFamilyIndex;
public:
    CommandPool(VkDevice device, VkCommandPool command_pool, uint32_t queue_family_index);
    ~CommandPool();

    CommandPool(CommandPool const&) = delete;
    CommandPool& operator=(CommandPool const&) = delete;

    CommandPool(CommandPool&& rhs);
    CommandPool& operator=(CommandPool&& rhs) = delete;

    uint32_t getQueueFamilyIndex() const;

    CommandBuffers allocateCommandBuffers(std::uint32_t command_buffer_count);

    void reset();
    void reset(VkCommandPoolResetFlags flags);
};
}
#endif
