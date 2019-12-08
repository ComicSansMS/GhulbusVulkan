#include <gbVk/CommandBuffer.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
CommandBuffer::CommandBuffer(VkCommandBuffer command_buffer)
    :m_commandBuffer(command_buffer), m_currentState(State::Initial)
{}

CommandBuffer::~CommandBuffer()
{}

CommandBuffer::CommandBuffer(CommandBuffer&& rhs)
    :m_commandBuffer(rhs.m_commandBuffer)
{
    rhs.m_commandBuffer = nullptr;
}

CommandBuffer::State CommandBuffer::getCurrentState() const
{
    return m_currentState;
}

void CommandBuffer::begin()
{
    begin(0);
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags)
{
    GHULBUS_PRECONDITION(m_currentState == State::Initial);
    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = nullptr;
    VkResult res = vkBeginCommandBuffer(m_commandBuffer, &begin_info);
    checkVulkanError(res, "Error in vkBeginCommandBuffer.");
    m_currentState = State::Recording;
}

void CommandBuffer::end()
{
    GHULBUS_PRECONDITION(m_currentState == State::Recording);
    VkResult res = vkEndCommandBuffer(m_commandBuffer);
    checkVulkanError(res, "Error in vkEndCommandBuffer.");
    m_currentState = State::Executable;
}

void CommandBuffer::reset()
{
    reset(0);
}

void CommandBuffer::reset(VkCommandBufferResetFlags flags)
{
    VkResult res = vkResetCommandBuffer(m_commandBuffer, flags);
    checkVulkanError(res, "Error in vkResetCommandBuffer.");
    m_currentState = State::Initial;
}

VkCommandBuffer CommandBuffer::getVkCommandBuffer()
{
    return m_commandBuffer;
}
}
