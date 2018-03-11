#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SWAPCHAIN_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SWAPCHAIN_HPP

/** @file
*
* @brief Swapchain.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <gbVk/Image.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <chrono>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Fence;
class ImageView;
class Semaphore;

class Swapchain {
public:
    class AcquiredImage
    {
        friend class Swapchain;
    private:
        Image* m_image;
        uint32_t m_swapchainIndex;
    private:
        AcquiredImage(Image& image, uint32_t swapchain_index);
    public:
        AcquiredImage();

        AcquiredImage(AcquiredImage const&) = delete;
        AcquiredImage& operator=(AcquiredImage const&) = delete;

        AcquiredImage(AcquiredImage&& rhs);
        AcquiredImage& operator=(AcquiredImage&&) = delete;

        Image* operator->();
        Image& operator*();

        operator bool() const;

        uint32_t getSwapchainIndex() const;
    };
private:
    VkSwapchainKHR m_swapchain;
    VkDevice m_device;
    std::vector<Image> m_images;
public:
    Swapchain(VkDevice logical_device, VkSwapchainKHR swapchain, VkExtent2D const& extent, VkFormat format);

    ~Swapchain();

    Swapchain(Swapchain const&) = delete;
    Swapchain& operator=(Swapchain const&) = delete;

    Swapchain(Swapchain&& rhs);
    Swapchain& operator=(Swapchain&&) = delete;

    VkSwapchainKHR getVkSwapchainKHR();

    uint32_t getNumberOfImages() const;

    std::vector<ImageView> getImageViews();

    std::vector<VkImage> getVkImages();

    AcquiredImage acquireNextImage(Fence& fence);

    AcquiredImage acquireNextImage(Fence& fence, std::chrono::nanoseconds timeout);

    AcquiredImage acquireNextImage(Semaphore& semaphore);

    AcquiredImage acquireNextImage(Semaphore& semaphore, std::chrono::nanoseconds timeout);

    AcquiredImage acquireNextImage(Fence& fence, Semaphore& semaphore);

    AcquiredImage acquireNextImage(Fence& fence, Semaphore& semaphore, std::chrono::nanoseconds timeout);

    void present(VkQueue queue, AcquiredImage&& image);

private:
    AcquiredImage acquireNextImage_impl(Fence* fence, Semaphore* semaphore, std::chrono::nanoseconds* timeout);
};
}
#endif
