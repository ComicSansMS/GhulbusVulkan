#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PHYSICAL_DEVICE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PHYSICAL_DEVICE_HPP

/** @file
*
* @brief Physical device.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <optional>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Device;
class DeviceBuilder;

struct NoSwapchainSupport_T {};
inline constexpr NoSwapchainSupport_T no_swapchain_support {};

class PhysicalDevice {
private:
    VkPhysicalDevice m_physicalDevice;
public:
    explicit PhysicalDevice(VkPhysicalDevice physical_device);

    VkPhysicalDevice getVkPhysicalDevice();

    VkPhysicalDeviceProperties getProperties();

    VkPhysicalDeviceFeatures getFeatures();

    VkPhysicalDeviceMemoryProperties getMemoryProperties();

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties();

    VkFormatProperties getFormatProperties(VkFormat format);

    std::optional<VkFormat> findSupportedFormat(std::vector<VkFormat> const& candidates,
                                                VkImageTiling tiling, VkFormatFeatureFlags features);

    std::optional<VkFormat> findDepthBufferFormat();

    VkImageFormatProperties getImageFormatProperties(VkFormat format, VkImageType type,
                                                     VkImageTiling tiling, VkImageUsageFlags usage,
                                                     VkImageCreateFlags create_flags);

    std::optional<uint32_t> findMemoryTypeIndex(VkMemoryPropertyFlags requested_properties);

    std::optional<uint32_t> findMemoryTypeIndex(VkMemoryPropertyFlags requested_properties,
                                                VkMemoryRequirements const& requirements);

    bool getSurfaceSupport(uint32_t queue_family, VkSurfaceKHR surface);

    [[deprecated]] std::vector<VkLayerProperties> enumerateDeviceLayerProperties();

    std::vector<VkExtensionProperties> enumerateDeviceExtensionProperties();

    std::vector<VkExtensionProperties> enumerateDeviceExtensionProperties(VkLayerProperties layer);

    DeviceBuilder createDeviceBuilder(NoSwapchainSupport_T const&);

    DeviceBuilder createDeviceBuilder();

    Device createDevice();
};
}
#endif
