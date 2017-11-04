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

        Image* operator->();
        Image& operator*();

        operator bool() const;
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

    std::vector<VkImage> getImages();

    AcquiredImage acquireNextImage(Fence& fence);

    AcquiredImage acquireNextImage(Fence& fence, std::chrono::nanoseconds timeout);

    void present(VkQueue queue, AcquiredImage&& image);
};
}
#endif
