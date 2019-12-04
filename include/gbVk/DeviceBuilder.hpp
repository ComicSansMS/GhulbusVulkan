#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_BUILDER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_BUILDER_HPP

/** @file
*
* @brief Device Builder.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <optional>
#include <string>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Device;

class DeviceBuilder {
public:
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::vector<std::vector<float>> queue_create_priorities;
    std::vector<std::string> extensions;
    std::optional<VkPhysicalDeviceFeatures> requested_features;
private:
    VkPhysicalDevice m_physicalDevice;
public:
    DeviceBuilder(VkPhysicalDevice physical_device);

    void addQueues(uint32_t queue_family, uint32_t n_queues_in_family);

    void addExtension(std::string extension);

    Device create();
};
}
#endif
