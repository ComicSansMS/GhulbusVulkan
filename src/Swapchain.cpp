#include <gbVk/Swapchain.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/Fence.hpp>

#include <gbBase/Assert.hpp>

#include <algorithm>
#include <iterator>
#include <limits>

namespace GHULBUS_VULKAN_NAMESPACE
{
Swapchain::AcquiredImage::AcquiredImage(Image& image, uint32_t swapchain_index)
    :m_image(&image), m_swapchainIndex(swapchain_index)
{
}

Swapchain::AcquiredImage::AcquiredImage()
    :m_image(nullptr), m_swapchainIndex(std::numeric_limits<decltype(m_swapchainIndex)>::max())
{
}

Swapchain::AcquiredImage::AcquiredImage(AcquiredImage&& rhs)
    :m_image(rhs.m_image), m_swapchainIndex(rhs.m_swapchainIndex)
{
    rhs.m_image = nullptr;
    rhs.m_swapchainIndex = std::numeric_limits<decltype(m_swapchainIndex)>::max();
}

Image* Swapchain::AcquiredImage::operator->()
{
    return m_image;
}

Image& Swapchain::AcquiredImage::operator*()
{
    return *m_image;
}

Swapchain::AcquiredImage::operator bool() const
{
    return m_image != nullptr;
}

Swapchain::Swapchain(VkDevice logical_device, VkSwapchainKHR swapchain, VkExtent2D const& extent, VkFormat format)
    :m_swapchain(swapchain), m_device(logical_device)
{
    uint32_t image_count;
    VkResult res = vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
    checkVulkanError(res, "Error in vkGetSwapchainImagesKHR.");

    std::vector<VkImage> sc_images;
    sc_images.resize(image_count);
    res = vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, sc_images.data());
    checkVulkanError(res, "Error in vkGetSwapchainImagesKHR.");
    GHULBUS_ASSERT_PRD(image_count == sc_images.size());

    m_images.reserve(image_count);
    VkExtent3D image_extent;
    image_extent.width = extent.width;
    image_extent.height = extent.height;
    image_extent.depth = 1;
    for(auto const& img : sc_images) {
        m_images.emplace_back(m_device, img, image_extent, format, Image::NoOwnership());
    }
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
    std::vector<VkImage> ret;
    ret.reserve(m_images.size());

    std::transform(begin(m_images), end(m_images), back_inserter(ret), [](Image& img) { return img.getVkImage(); });
    return ret;
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Fence& fence)
{
    return acquireNextImage(fence, std::chrono::nanoseconds::max());
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Fence& fence, std::chrono::nanoseconds timeout)
{
    uint32_t index;
    VkResult res = vkAcquireNextImageKHR(m_device, m_swapchain, timeout.count(), nullptr, fence.getVkFence(), &index);
    if(res == VK_NOT_READY) { return AcquiredImage(); }
    checkVulkanError(res, "Error in vkAcquireNextImageKHR.");
    GHULBUS_ASSERT((index >= 0) && (index < m_images.size()));
    return AcquiredImage(m_images[index], index);
}

void Swapchain::present(VkQueue queue, AcquiredImage&& image)
{
    GHULBUS_PRECONDITION_PRD(image);
    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 0;
    present_info.pWaitSemaphores = nullptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain;
    present_info.pImageIndices = &image.m_swapchainIndex;
    present_info.pResults = nullptr;
    VkResult res = vkQueuePresentKHR(queue, &present_info);
    checkVulkanError(res, "Error in vkQueuePresentKHR.");
}
}
