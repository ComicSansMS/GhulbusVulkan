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

#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class PhysicalDevice {
private:
    VkPhysicalDevice m_physical_device;
public:
    PhysicalDevice(VkPhysicalDevice physical_device);

    VkPhysicalDeviceProperties getProperties();

    VkPhysicalDeviceFeatures getFeatures();

    VkPhysicalDeviceMemoryProperties getMemoryProperties();

    std::vector<VkQueueFamilyProperties> getQueueFamilyProperties();

    std::vector<VkLayerProperties> enumerateDeviceLayerProperties();

    std::vector<VkExtensionProperties> enumerateDeviceExtensionProperties();

    std::vector<VkExtensionProperties> enumerateDeviceExtensionProperties(VkLayerProperties layer);

    VkDevice createDevice();
};
}
#endif
