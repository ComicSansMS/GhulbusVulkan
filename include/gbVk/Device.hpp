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

#include <gbVk/DeviceMemory.hpp>
#include <gbVk/DeviceMemoryAllocator_Trivial.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Buffer;
class CommandPool;
class DescriptorPoolBuilder;
class DescriptorSetLayoutBuilder;
class Event;
class Fence;
class Framebuffer;
class Image;
class ImageView;
class PhysicalDevice;
class PipelineBuilder;
class PipelineLayoutBuilder;
class Queue;
class RenderPass;
class RenderPassBuilder;
class Sampler;
class Semaphore;
class ShaderModule;
class SpirvCode;
class Swapchain;

class Device {
private:
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
    DeviceMemoryAllocator_Trivial m_allocator;
public:
    Device(VkPhysicalDevice physical_device, VkDevice logical_device);

    ~Device();

    Device(Device const&) = delete;
    Device& operator=(Device const&) = delete;

    Device(Device&& rhs);
    Device& operator=(Device&&) = delete;

    VkDevice getVkDevice();

    PhysicalDevice getPhysicalDevice();

    Swapchain createSwapchain(VkSurfaceKHR surface, uint32_t queue_family);
    Swapchain createSwapchain(VkSurfaceKHR surface, uint32_t queue_family, VkSwapchainKHR old_swapchain);

    Fence createFence();

    Fence createFence(VkFenceCreateFlags flags);

    Semaphore createSemaphore();

    Event createEvent();

    Buffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags);

    Buffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode);

    Image createImage2D(uint32_t width, uint32_t height);
    Image createImage2D(uint32_t width, uint32_t height, VkFormat format);
    Image createImageDepthBuffer(uint32_t width, uint32_t height, VkFormat format);
    Image createImage(VkExtent3D const& extent, VkFormat format, uint32_t mip_levels, uint32_t array_layers,
                      VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage_flags);

    Sampler createSampler();

    Sampler createSampler(VkFilter min_mag_filter, VkSamplerAddressMode address_mode, float max_anisotropy);

    Sampler createSampler(VkFilter min_filter, VkFilter mag_filter, VkSamplerMipmapMode mipmap_mode,
                          VkSamplerAddressMode address_mode_u, VkSamplerAddressMode address_mode_v,
                          VkSamplerAddressMode address_mode_w, float mip_lod_bias, float max_anisotropy,
                          float min_lod, float max_lod, VkBorderColor border_color);

    DeviceMemory allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags);

    DeviceMemory allocateMemory(VkMemoryRequirements const& requirements, VkMemoryPropertyFlags required_flags);

    CommandPool createCommandPool(VkCommandPoolCreateFlags requested_flags, uint32_t queue_family_index);

    Queue getQueue(uint32_t queue_family, uint32_t queue_index);

    ShaderModule createShaderModule(SpirvCode const& code);

    std::vector<Framebuffer> createFramebuffers(Swapchain& swapchain, RenderPass& render_pass);

    std::vector<Framebuffer> createFramebuffers(Swapchain& swapchain, RenderPass& render_pass,
                                                ImageView& depth_stencil);

    RenderPassBuilder createRenderPassBuilder();

    DescriptorSetLayoutBuilder createDescriptorSetLayoutBuilder();

    DescriptorPoolBuilder createDescriptorPoolBuilder();

    PipelineLayoutBuilder createPipelineLayoutBuilder();

    PipelineBuilder createGraphicsPipelineBuilder(uint32_t viewport_width, uint32_t viewport_height);

    void waitIdle();
};
}
#endif
