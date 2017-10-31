#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_HPP

/** @file
*
* @brief Logical device.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class CommandPool;
class DeviceMemory;
class PhysicalDevice;
class Swapchain;

class Device {
private:
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
public:
    Device(VkPhysicalDevice physical_device, VkDevice logical_device);

    ~Device();

    Device(Device const&) = delete;
    Device& operator=(Device const&) = delete;

    Device(Device&& rhs);
    Device& operator=(Device&&) = delete;

    VkDevice getVkDevice();

    PhysicalDevice getPhysicalDevice();

    Swapchain createSwapChain(VkSurfaceKHR surface, uint32_t queue_family);

    DeviceMemory allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags);

    CommandPool createCommandPool(VkCommandPoolCreateFlags requested_flags, uint32_t queue_family_index);

    VkQueue getQueue(uint32_t queue_family, uint32_t queue_index);

    void waitIdle();
};
}
#endif
