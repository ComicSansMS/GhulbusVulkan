#include <gbVk/CommandPool.hpp>

#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
CommandPool::CommandPool(VkDevice device, VkCommandPool command_pool)
    :m_commandPool(command_pool), m_device(device)
{
}

CommandPool::~CommandPool()
{
    if(m_commandPool) { vkDestroyCommandPool(m_device, m_commandPool, nullptr); }
}

CommandPool::CommandPool(CommandPool&& rhs)
    :m_commandPool(rhs.m_commandPool), m_device(rhs.m_device)
{
    rhs.m_commandPool = nullptr;
    rhs.m_device = nullptr;
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
    return CommandBuffers(m_device, m_commandPool, std::move(buffers));
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
