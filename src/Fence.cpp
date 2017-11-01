#include <gbVk/Fence.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Fence::Fence(VkDevice logical_device, VkFence fence)
    :m_fence(fence), m_device(logical_device)
{
}

Fence::~Fence()
{
    if(m_fence) { vkDestroyFence(m_device, m_fence, nullptr); }
}

Fence::Fence(Fence&& rhs)
    :m_fence(rhs.m_fence), m_device(rhs.m_device)
{
    rhs.m_fence = nullptr;
    rhs.m_device = nullptr;
}

VkFence Fence::getVkFence()
{
    return m_fence;
}

Fence::Status Fence::getStatus()
{
    VkResult res = vkGetFenceStatus(m_device, m_fence);
    if(res == VK_NOT_READY) { return Status::NotReady; }
    checkVulkanError(res, "Error in vkGetFenceStatus.");
    return Status::Ready;
}

void Fence::wait()
{
    auto const status = wait_for(std::chrono::nanoseconds::max());
    GHULBUS_ASSERT(status == Status::Ready);
}

Fence::Status Fence::wait_for(std::chrono::nanoseconds timeout)
{
    VkResult res = vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout.count());
    if(res == VK_NOT_READY) { return Status::NotReady; }
    checkVulkanError(res, "Error in vkWaitForFences.");
    return Status::Ready;
}

void Fence::reset()
{
    VkResult res = vkResetFences(m_device, 1, &m_fence);
    checkVulkanError(res, "Error in vkResetFences.");
}
}
