#include <gbVk/Swapchain.hpp>

#include <gbVk/Device.hpp>
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

Swapchain::AcquiredImage& Swapchain::AcquiredImage::operator=(AcquiredImage&& rhs)
{
    if (&rhs != this) {
        m_image = rhs.m_image;
        m_swapchainIndex = rhs.m_swapchainIndex;
        rhs.m_image = nullptr;
        rhs.m_swapchainIndex = std::numeric_limits<decltype(m_swapchainIndex)>::max();
    }
    return *this;
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

Swapchain::Swapchain(VkDevice logical_device, VkSwapchainKHR swapchain, VkExtent2D const& extent, VkFormat format,
                     VkSurfaceKHR surface, uint32_t queue_family)
    :m_swapchain(swapchain), m_device(logical_device), m_surface(surface), m_queueFamily(queue_family)
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
    :m_swapchain(rhs.m_swapchain), m_device(rhs.m_device), m_images(std::move(rhs.m_images)),
     m_surface(rhs.m_surface), m_queueFamily(rhs.m_queueFamily)
{
    rhs.m_swapchain = nullptr;
    rhs.m_device = nullptr;
    rhs.m_surface = nullptr;
    rhs.m_queueFamily = VK_QUEUE_FAMILY_IGNORED;
}

Swapchain& Swapchain::operator=(Swapchain&& rhs)
{
    if (&rhs != this) {
        if(m_swapchain) { vkDestroySwapchainKHR(m_device, m_swapchain, nullptr); }
        m_swapchain = rhs.m_swapchain;
        m_device = rhs.m_device;
        m_images = std::move(rhs.m_images);
        m_surface = rhs.m_surface;
        m_queueFamily = rhs.m_queueFamily;
        rhs.m_swapchain = nullptr;
        rhs.m_device = nullptr;
        rhs.m_surface = nullptr;
        rhs.m_queueFamily = VK_QUEUE_FAMILY_IGNORED;
    }
    return *this;
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

VkFormat Swapchain::getFormat() const
{
    GHULBUS_PRECONDITION(!m_images.empty());
    return m_images.front().getFormat();
}

std::vector<ImageView> Swapchain::createImageViews()
{
    std::vector<ImageView> ret;
    ret.reserve(m_images.size());
    for(std::size_t i = 0; i < m_images.size(); ++i) {
        ret.emplace_back(m_images[i].createImageView());
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

void Swapchain::recreate(GhulbusVulkan::Device& device)
{
    Swapchain new_swapchain = device.createSwapchain(m_surface, m_queueFamily, m_swapchain);
    *this = std::move(new_swapchain);
}
}
