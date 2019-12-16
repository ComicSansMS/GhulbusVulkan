#include <gbVk/CommandPool.hpp>

#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
CommandPool::CommandPool(VkDevice device, VkCommandPool command_pool, uint32_t queue_family_index)
    :m_commandPool(command_pool), m_device(device), m_queueFamilyIndex(queue_family_index)
{
}

CommandPool::~CommandPool()
{
    if(m_commandPool) { vkDestroyCommandPool(m_device, m_commandPool, nullptr); }
}

CommandPool::CommandPool(CommandPool&& rhs)
    :m_commandPool(rhs.m_commandPool), m_device(rhs.m_device), m_queueFamilyIndex(rhs.m_queueFamilyIndex)
{
    rhs.m_commandPool = nullptr;
    rhs.m_device = nullptr;
    rhs.m_queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
}

uint32_t CommandPool::getQueueFamilyIndex() const
{
    return m_queueFamilyIndex;
}

CommandBuffers CommandPool::allocateCommandBuffers(std::uint32_t command_buffer_count)
{
    GHULBUS_PRECONDITION_DBG(command_buffer_count > 0);
    VkCommandBufferAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.commandPool = m_commandPool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = command_buffer_count;
    std::vector<VkCommandBuffer> buffers;
    buffers.resize(command_buffer_count);
    VkResult res = vkAllocateCommandBuffers(m_device, &alloc_info, buffers.data());
    checkVulkanError(res, "Error in vkAllocateCommandBuffers.");
    return CommandBuffers(m_device, m_commandPool, std::move(buffers), m_queueFamilyIndex);
}

void CommandPool::reset()
{
    reset(0);
}

void CommandPool::reset(VkCommandPoolResetFlags flags)
{
    VkResult res = vkResetCommandPool(m_device, m_commandPool, flags);
    checkVulkanError(res, "Error in vkResetCommandPool.");
}
}
