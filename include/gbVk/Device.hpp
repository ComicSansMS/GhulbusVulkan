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
class Device {
private:
    VkDevice m_device;
public:
    explicit Device(VkDevice logical_device);

    ~Device();

    Device(Device const&) = delete;
    Device& operator=(Device const&) = delete;

    Device(Device&&);
    Device& operator=(Device&&) = delete;
};
}
#endif
