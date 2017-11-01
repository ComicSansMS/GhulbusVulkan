#include <gbVk/Swapchain.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Swapchain::Swapchain(VkDevice logical_device, VkSwapchainKHR swapchain)
    :m_swapchain(swapchain), m_device(logical_device)
{
}

Swapchain::~Swapchain()
{
    if(m_swapchain) { vkDestroySwapchainKHR(m_device, m_swapchain, nullptr); }
}

Swapchain::Swapchain(Swapchain&& rhs)
    :m_swapchain(rhs.m_swapchain), m_device(rhs.m_device)
{
    rhs.m_swapchain = nullptr;
    rhs.m_device = nullptr;
}

VkSwapchainKHR Swapchain::getVkSwapchainKHR()
{
    return m_swapchain;
}

std::vector<VkImage> Swapchain::getImages()
{
    uint32_t image_count;
    VkResult res = vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
    checkVulkanError(res, "Error in vkGetSwapchainImagesKHR.");

    std::vector<VkImage> ret;
    ret.resize(image_count);
    res = vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, ret.data());
    checkVulkanError(res, "Error in vkGetSwapchainImagesKHR.");
    GHULBUS_ASSERT_PRD(image_count == ret.size());

    return ret;
}

std::optional<uint32_t> Swapchain::acquireNextImage()
{
    return acquireNextImage(std::chrono::milliseconds(0));
}

std::optional<uint32_t> Swapchain::acquireNextImage(std::chrono::nanoseconds timeout)
{
    uint32_t ret;
    VkResult res = vkAcquireNextImageKHR(m_device, m_swapchain, timeout.count(), nullptr, nullptr, &ret);
    if(res == VK_NOT_READY) { return std::nullopt; }
    checkVulkanError(res, "Error in vkAcquireNextImageKHR.");
    return ret;
}
}
