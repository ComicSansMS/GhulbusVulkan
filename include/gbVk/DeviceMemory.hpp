#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_HPP

/** @file
*
* @brief Device Memory.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DeviceMemory {
private:
    VkDeviceMemory m_memory;
    VkDevice m_device;
public:
    DeviceMemory(VkDevice device, VkDeviceMemory memory);

    ~DeviceMemory();

    DeviceMemory(DeviceMemory const&) = delete;
    DeviceMemory& operator=(DeviceMemory const&) = delete;

    DeviceMemory(DeviceMemory&&);
    DeviceMemory& operator=(DeviceMemory&&) = delete;

    VkDeviceSize getCommitment();
};
}
#endif
