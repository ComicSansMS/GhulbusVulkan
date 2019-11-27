#include <gbVk/Swapchain.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/ImageView.hpp>
#include <gbVk/Semaphore.hpp>

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

uint32_t Swapchain::AcquiredImage::getSwapchainIndex() const
{
    return m_swapchainIndex;
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

uint32_t Swapchain::getNumberOfImages() const
{
    return static_cast<uint32_t>(m_images.size());
}

uint32_t Swapchain::getWidth() const
{
    GHULBUS_PRECONDITION(!m_images.empty());
    return m_images.front().getWidth();
}

uint32_t Swapchain::getHeight() const
{
    GHULBUS_PRECONDITION(!m_images.empty());
    return m_images.front().getHeight();
}

std::vector<ImageView> Swapchain::getImageViews()
{
    std::vector<ImageView> ret;
    ret.reserve(m_images.size());
    for(std::size_t i = 0; i < m_images.size(); ++i) {
        VkImageViewCreateInfo image_view_ci;
        image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_ci.pNext = nullptr;
        image_view_ci.flags = 0;
        image_view_ci.image = m_images[i].getVkImage();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.format = m_images[i].getFormat();
        image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.baseMipLevel = 0;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = 1;
        VkImageView image_view;
        VkResult res = vkCreateImageView(m_device, &image_view_ci, nullptr, &image_view);
        checkVulkanError(res, "Error in vkCreateImageView.");
        ret.emplace_back(m_device, image_view);
    }
    return ret;
}

std::vector<VkImage> Swapchain::getVkImages()
{
    std::vector<VkImage> ret;
    ret.reserve(m_images.size());

    std::transform(begin(m_images), end(m_images), back_inserter(ret), [](Image& img) { return img.getVkImage(); });
    return ret;
}

Swapchain::AcquiredImage Swapchain::acquireNextImage_impl(Fence* fence, Semaphore* semaphore,
                                               std::chrono::nanoseconds* timeout)
{
    uint32_t index;
    VkResult res = vkAcquireNextImageKHR(m_device, m_swapchain,
        (timeout   ? timeout->count()            : std::numeric_limits<uint64_t>::max()),
        (semaphore ? semaphore->getVkSemaphore() : VK_NULL_HANDLE),
        (fence     ? fence->getVkFence()         : VK_NULL_HANDLE),
        &index);
    if(res == VK_NOT_READY) { return AcquiredImage(); }
    checkVulkanError(res, "Error in vkAcquireNextImageKHR.");
    GHULBUS_ASSERT((index >= 0) && (index < m_images.size()));
    return AcquiredImage(m_images[index], index);
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Fence& fence)
{
    return acquireNextImage_impl(&fence, nullptr, nullptr);
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Fence& fence, std::chrono::nanoseconds timeout)
{
    return acquireNextImage_impl(&fence, nullptr, &timeout);
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Semaphore& semaphore)
{
    return acquireNextImage_impl(nullptr, &semaphore, nullptr);
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Semaphore& semaphore, std::chrono::nanoseconds timeout)
{
    return acquireNextImage_impl(nullptr, &semaphore, &timeout);
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Fence& fence, Semaphore& semaphore)
{
    return acquireNextImage_impl(&fence, &semaphore, nullptr);
}

Swapchain::AcquiredImage Swapchain::acquireNextImage(Fence& fence, Semaphore& semaphore,
                                                     std::chrono::nanoseconds timeout)
{
    return acquireNextImage_impl(&fence, &semaphore, &timeout);
}

void Swapchain::present(VkQueue queue, AcquiredImage&& image)
{
    present_impl(queue, nullptr, std::move(image));
}

void Swapchain::present(VkQueue queue, Semaphore& semaphore, AcquiredImage&& image)
{
    present_impl(queue, &semaphore, std::move(image));
}

void Swapchain::present_impl(VkQueue queue, Semaphore* semaphore, AcquiredImage&& image)
{
    GHULBUS_PRECONDITION_PRD(image);
    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    VkSemaphore semaph = (semaphore ? semaphore->getVkSemaphore() : VK_NULL_HANDLE);
    present_info.waitSemaphoreCount = (semaphore ? 1 : 0);
    present_info.pWaitSemaphores = (semaphore ? &semaph : nullptr);
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swapchain;
    present_info.pImageIndices = &image.m_swapchainIndex;
    present_info.pResults = nullptr;
    VkResult res = vkQueuePresentKHR(queue, &present_info);
    checkVulkanError(res, "Error in vkQueuePresentKHR.");
}
}
