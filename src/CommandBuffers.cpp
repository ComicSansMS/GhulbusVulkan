#include <gbVk/CommandBuffers.hpp>

#include <gbVk/CommandBuffer.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <limits>

namespace GHULBUS_VULKAN_NAMESPACE
{
CommandBuffers::CommandBuffers(VkDevice logical_device, VkCommandPool command_pool,
                               std::vector<VkCommandBuffer> command_buffers)
    :m_commandBuffers(std::move(command_buffers)), m_device(logical_device), m_commandPool(command_pool)
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
    :m_commandBuffers(std::move(rhs.m_commandBuffers)), m_device(rhs.m_device), m_commandPool(rhs.m_commandPool)
{
    rhs.m_commandBuffers.clear();
}

uint32_t CommandBuffers::size() const
{
    return static_cast<uint32_t>(m_commandBuffers.size());
}

CommandBuffer CommandBuffers::getCommandBuffer(uint32_t index)
{
    GHULBUS_PRECONDITION_DBG((index >= 0) && (index < size()));
    return CommandBuffer(m_commandBuffers[index]);
}

VkCommandBuffer* CommandBuffers::getVkCommandBuffers()
{
    return m_commandBuffers.data();
}
}
