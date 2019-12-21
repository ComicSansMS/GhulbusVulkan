#include <gbVk/CommandBuffers.hpp>

#include <gbVk/CommandBuffer.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <limits>

namespace GHULBUS_VULKAN_NAMESPACE
{
CommandBuffers::CommandBuffers()
    :m_device(VK_NULL_HANDLE), m_commandPool(VK_NULL_HANDLE), m_queueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
{}

CommandBuffers::CommandBuffers(VkDevice logical_device, VkCommandPool command_pool,
                               std::vector<VkCommandBuffer> command_buffers, uint32_t queue_family_index)
    :m_commandBuffers(std::move(command_buffers)), m_device(logical_device), m_commandPool(command_pool),
    m_queueFamilyIndex(queue_family_index)
{
    GHULBUS_PRECONDITION(m_commandBuffers.size() < std::numeric_limits<uint32_t>::max());
    m_commandBufferObjects.reserve(m_commandBuffers.size());
    for (auto const& command_buffer : m_commandBuffers) {
        m_commandBufferObjects.emplace_back(command_buffer, m_queueFamilyIndex);
    }
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
     m_queueFamilyIndex(rhs.m_queueFamilyIndex), m_commandBufferObjects(std::move(rhs.m_commandBufferObjects))
{
    rhs.m_commandBuffers.clear();
    rhs.m_queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
}

CommandBuffers& CommandBuffers::operator=(CommandBuffers&& rhs)
{
    if (&rhs != this) {
        if(!m_commandBuffers.empty())
        {
            vkFreeCommandBuffers(m_device, m_commandPool,
                static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        }
        m_commandBuffers = std::move(rhs.m_commandBuffers);
        m_device = rhs.m_device;
        m_commandPool = rhs.m_commandPool;
        m_queueFamilyIndex = rhs.m_queueFamilyIndex;
        m_commandBufferObjects = std::move(rhs.m_commandBufferObjects);
        rhs.m_device = VK_NULL_HANDLE;
        rhs.m_commandPool = VK_NULL_HANDLE;
        rhs.m_queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }
    return *this;
}

uint32_t CommandBuffers::size() const
{
    return static_cast<uint32_t>(m_commandBuffers.size());
}

uint32_t CommandBuffers::getQueueFamilyIndex() const
{
    return m_queueFamilyIndex;
}

CommandBuffer& CommandBuffers::getCommandBuffer(uint32_t index)
{
    GHULBUS_PRECONDITION_DBG((index >= 0) && (index < size()));
    GHULBUS_ASSERT(m_commandBuffers.size() == m_commandBufferObjects.size());
    return m_commandBufferObjects[index];
}

VkCommandBuffer* CommandBuffers::getVkCommandBuffers()
{
    GHULBUS_PRECONDITION(!m_commandBuffers.empty());
    return m_commandBuffers.data();
}
}
