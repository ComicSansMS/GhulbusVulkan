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
class Buffer;
class CommandPool;
class DeviceMemory;
class Fence;
class Image;
class PhysicalDevice;
class RenderPassBuilder;
class Semaphore;
class ShaderModule;
class Swapchain;
namespace Spirv
{
class Code;
}

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

    Fence createFence();

    Fence createFence(VkFenceCreateFlags flags);

    Semaphore createSemaphore();

    Buffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags);

    Image createImage(uint32_t width, uint32_t height);
    Image createImage(VkExtent3D const& extent, VkFormat format);
    Image createImage(VkExtent3D const& extent, VkFormat format, uint32_t mip_levels, uint32_t array_layers,
                      VkImageTiling tiling, VkImageUsageFlags usage_flags);

    DeviceMemory allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags);

    DeviceMemory allocateMemory(size_t requested_size, VkMemoryPropertyFlags required_flags,
                                VkMemoryRequirements const& requirements);

    CommandPool createCommandPool(VkCommandPoolCreateFlags requested_flags, uint32_t queue_family_index);

    VkQueue getQueue(uint32_t queue_family, uint32_t queue_index);

    ShaderModule createShaderModule(Spirv::Code const& code);

    RenderPassBuilder createRenderPassBuilder();

    void waitIdle();
};
}
#endif
