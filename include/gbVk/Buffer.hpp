#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_BUFFER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_BUFFER_HPP

/** @file
*
* @brief Buffer.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DeviceMemory;

class Buffer {
private:
    VkBuffer m_buffer;
    VkDevice m_device;
public:
    Buffer(VkDevice logical_device, VkBuffer buffer);
    ~Buffer();

    Buffer(Buffer const&) = delete;
    Buffer& operator=(Buffer const&) = delete;

    Buffer(Buffer&& rhs);
    Buffer& operator=(Buffer&& rhs) = delete;

    VkBuffer getVkBuffer();

    VkMemoryRequirements getMemoryRequirements();

    void bindBufferMemory(DeviceMemory& memory);

    void bindBufferMemory(DeviceMemory& memory, VkDeviceSize offset);
};
}
#endif
