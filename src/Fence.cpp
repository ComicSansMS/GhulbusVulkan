#include <gbVk/Fence.hpp>

#include <gbVk/DebugUtilsObjectName.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <algorithm>
#include <limits>

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

void Fence::setDebugName(char const* name)
{
    DebugUtils::setObjectName(m_device, name, m_fence, VK_OBJECT_TYPE_FENCE);
}

VkFence Fence::getVkFence()
{
    return m_fence;
}

Fence::Status Fence::getStatus()
{
    VkResult const res = vkGetFenceStatus(m_device, m_fence);
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
    VkResult const res = vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout.count());
    if(res == VK_TIMEOUT) { return Status::NotReady; }
    checkVulkanError(res, "Error in vkWaitForFences.");
    return Status::Ready;
}

void Fence::reset()
{
    VkResult res = vkResetFences(m_device, 1, &m_fence);
    checkVulkanError(res, "Error in vkResetFences.");
}

/* static */ std::size_t Fence::wait_impl(std::span<VkDevice> vk_devices,
                                          std::span<VkFence> vk_fences,
                                          std::chrono::nanoseconds timeout,
                                          bool wait_all)
{
    GHULBUS_ASSERT(vk_devices.size() == vk_fences.size());
    if (!(std::all_of(begin(vk_devices), end(vk_devices),
                      [&](VkDevice d) { return d == vk_devices.front(); })))
    {
        GHULBUS_THROW(Exceptions::VulkanError(), "Attempt to wait on fences from different devices");
    }
    VkResult const res = vkWaitForFences(
        vk_devices.front(),
        static_cast<uint32_t>(vk_fences.size()),
        vk_fences.data(),
        (wait_all) ? VK_TRUE : VK_FALSE,
        timeout.count()
    );
    if (res == VK_TIMEOUT) { return std::numeric_limits<std::size_t>::max(); }
    checkVulkanError(res, "Error in vkWaitForFences.");
    if (!wait_all) {
        // check which fence is ready
        for (std::size_t i = 0; i < vk_devices.size(); ++i) {
            VkResult const res_i = vkGetFenceStatus(vk_devices[i], vk_fences[i]);
            if (res_i == VK_SUCCESS) { return i; }
            checkVulkanError(res_i, "Error in vkGetFenceStatus.");
        }
        GHULBUS_UNREACHABLE_MESSAGE("No fence is ready even though vkWaitForFences returned");
    }
    return 0;
}

}
