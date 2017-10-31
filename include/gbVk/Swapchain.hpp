#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SWAPCHAIN_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SWAPCHAIN_HPP

/** @file
*
* @brief Swapchain.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Swapchain {
private:
    VkSwapchainKHR m_swapchain;
    VkDevice m_device;
public:
    Swapchain(VkDevice logical_device, VkSwapchainKHR swapchain);

    ~Swapchain();

    Swapchain(Swapchain const&) = delete;
    Swapchain& operator=(Swapchain const&) = delete;

    Swapchain(Swapchain&&);
    Swapchain& operator=(Swapchain&&) = delete;

    VkSwapchainKHR getVkSwapchainKHR();

    std::vector<VkImage> getImages();
};
}
#endif
