#include <gbVk/CommandBuffers.hpp>

#include <gbVk/CommandBuffer.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <limits>

namespace GHULBUS_VULKAN_NAMESPACE
{
CommandBuffers::CommandBuffers(VkDevice logical_device, VkCommandPool command_pool,
                               std::vector<VkCommandBuffer> command_buffers, uint32_t queue_family_index)
    :m_commandBuffers(std::move(command_buffers)), m_device(logical_device), m_commandPool(command_pool),
    m_queueFamilyIndex(queue_family_index)
{
    GHULBUS_PRECONDITION(m_commandBuffers.size() < std::numeric_limits<uint32_t>::max());
}

CommandBuffers::~CommandBuffers()
{
    if(!m_commandBuffers.empty())
    {
        vkFreeCommandBuffers(m_device, m_commandPool,
                             static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    }
}

CommandBuffers::CommandBuffers(CommandBuffers&& rhs)
    :m_commandBuffers(std::move(rhs.m_commandBuffers)), m_device(rhs.m_device), m_commandPool(rhs.m_commandPool),
     m_queueFamilyIndex(rhs.m_queueFamilyIndex)
{
    rhs.m_commandBuffers.clear();
    rhs.m_queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
}

uint32_t CommandBuffers::size() const
{
    return static_cast<uint32_t>(m_commandBuffers.size());
}

uint32_t CommandBuffers::getQueueFamilyIndex() const
{
    return m_queueFamilyIndex;
}

CommandBuffer CommandBuffers::getCommandBuffer(uint32_t index)
{
    GHULBUS_PRECONDITION_DBG((index >= 0) && (index < size()));
    return CommandBuffer(m_commandBuffers[index], m_queueFamilyIndex);
}

VkCommandBuffer* CommandBuffers::getVkCommandBuffers()
{
    return m_commandBuffers.data();
}
}
