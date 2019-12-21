#include <gbVk/Device.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/CommandPool.hpp>
#include <gbVk/DescriptorPoolBuilder.hpp>
#include <gbVk/DescriptorSetLayoutBuilder.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Event.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/Framebuffer.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/ImageView.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/PipelineBuilder.hpp>
#include <gbVk/PipelineLayoutBuilder.hpp>
#include <gbVk/Queue.hpp>
#include <gbVk/RenderPass.hpp>
#include <gbVk/RenderPassBuilder.hpp>
#include <gbVk/Sampler.hpp>
#include <gbVk/Semaphore.hpp>
#include <gbVk/ShaderModule.hpp>
#include <gbVk/SpirvCode.hpp>
#include <gbVk/Swapchain.hpp>

#include <gbBase/Assert.hpp>

#include <utility>

namespace GHULBUS_VULKAN_NAMESPACE
{
Device::Device(VkPhysicalDevice physical_device, VkDevice logical_device)
    :m_device(logical_device), m_physicalDevice(physical_device), m_allocator(logical_device, physical_device)
{
}

Device::~Device()
{
    if(m_device) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
    }
}

Device::Device(Device&& rhs)
    :m_device(rhs.m_device), m_physicalDevice(rhs.m_physicalDevice), m_allocator(std::move(rhs.m_allocator))
{
    rhs.m_device = nullptr;
    rhs.m_physicalDevice = nullptr;
}

VkDevice Device::getVkDevice()
{
    return m_device;
}

PhysicalDevice Device::getPhysicalDevice()
{
    return PhysicalDevice(m_physicalDevice);
}

Swapchain Device::createSwapchain(VkSurfaceKHR surface, uint32_t queue_family)
{
    return createSwapchain(surface, queue_family, nullptr);
}

Swapchain Device::createSwapchain(VkSurfaceKHR surface, uint32_t queue_family, VkSwapchainKHR old_swapchain)
{
    VkBool32 is_supported;
    VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, queue_family, surface, &is_supported);
    checkVulkanError(res, "Error in vkGetPhysicalDeviceSurfaceSupportKHR.");
    GHULBUS_PRECONDITION_MESSAGE(is_supported, "Surface does not support presentation.");

    VkSurfaceCapabilitiesKHR surf_caps;
    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, surface, &surf_caps);
    checkVulkanError(res, "Error in vkGetPhysicalDeviceSurfaceCapabilitiesKHR.");

    uint32_t surf_fmt_count;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &surf_fmt_count, nullptr);
    checkVulkanError(res, "Error in vkGetPhysicalDeviceSurfaceFormatsKHR.");
    std::vector<VkSurfaceFormatKHR> surf_fmts;
    surf_fmts.resize(surf_fmt_count);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &surf_fmt_count, surf_fmts.data());
    checkVulkanError(res, "Error in vkGetPhysicalDeviceSurfaceFormatsKHR.");
    GHULBUS_ASSERT_PRD(surf_fmt_count == surf_fmts.size());
    auto const& surface_format = [&surf_fmts]() -> VkSurfaceFormatKHR const& {
            for(auto const& fmt : surf_fmts) {
                GHULBUS_ASSERT(fmt.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR);
                if(fmt.format == VK_FORMAT_B8G8R8A8_UNORM) {
                    return fmt;
                }
            }
            GHULBUS_THROW(Exceptions::ProtocolViolation(), "No suitable surface format found.");
        }();

    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.surface = surface;
    create_info.minImageCount = std::max(surf_caps.minImageCount, std::min(surf_caps.maxImageCount, 2u));
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent.width = surf_caps.currentExtent.width;
    create_info.imageExtent.height = surf_caps.currentExtent.height;
    create_info.imageArrayLayers = std::min(surf_caps.maxImageArrayLayers, 1u);
    if(((surf_caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) ||
       ((surf_caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0))
    {
        GHULBUS_THROW(Exceptions::ProtocolViolation(), "Requested image usage not supported.");
    }
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    if((surf_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == 0) {
        GHULBUS_THROW(Exceptions::ProtocolViolation(), "Requested transform mode not supported.");
    }
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    if((surf_caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0) {
        GHULBUS_THROW(Exceptions::ProtocolViolation(), "Requested alpha composite mode not supported.");
    }
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = VK_FALSE;
    create_info.oldSwapchain = old_swapchain;

    VkSwapchainKHR swapchain;
    res = vkCreateSwapchainKHR(m_device, &create_info, nullptr, &swapchain);
    checkVulkanError(res, "Error in vkCreateSwapchainKHR.");

    return Swapchain(m_device, swapchain, create_info.imageExtent, create_info.imageFormat, surface, queue_family);
}

Fence Device::createFence()
{
    return createFence(0);
}

Fence Device::createFence(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    VkFence fence;
    VkResult res = vkCreateFence(m_device, &create_info, nullptr, &fence);
    checkVulkanError(res, "Error in vkCreateFence.");
    return Fence(m_device, fence);
}

Semaphore Device::createSemaphore()
{
    VkSemaphoreCreateInfo semaphore_ci;
    semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_ci.pNext = nullptr;
    semaphore_ci.flags = 0;
    VkSemaphore semaphore;
    VkResult res = vkCreateSemaphore(m_device, &semaphore_ci, nullptr, &semaphore);
    checkVulkanError(res, "Error in vkCreateSemaphore.");
    return Semaphore(m_device, semaphore);
}

Event Device::createEvent()
{
    VkEventCreateInfo event_ci;
    event_ci.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
    event_ci.pNext = nullptr;
    event_ci.flags = 0;
    VkEvent event_v;
    VkResult const res = vkCreateEvent(m_device, &event_ci, nullptr, &event_v);
    checkVulkanError(res, "Error in vkCreateEvent.");
    return Event(m_device, event_v);
}

Buffer Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags)
{
    return createBuffer(size, usage_flags, VK_SHARING_MODE_EXCLUSIVE);
}

Buffer Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags, VkSharingMode sharing_mode)
{
    VkBufferCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.size = size;
    create_info.usage = usage_flags;
    create_info.sharingMode = sharing_mode;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    VkBuffer buffer;
    VkResult res = vkCreateBuffer(m_device, &create_info, nullptr, &buffer);
    checkVulkanError(res, "Error in vkCreateBuffer.");
    return Buffer(m_device, buffer);
}

Image Device::createImage2D(uint32_t width, uint32_t height)
{
    return createImage2D(width, height, VK_FORMAT_R8G8B8A8_UNORM);
}

Image Device::createImage2D(uint32_t width, uint32_t height, VkFormat format)
{
    VkExtent3D extent;
    extent.width = width;
    extent.height = height;
    extent.depth = 1;
    return createImage(extent, format, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
}

Image Device::createImageDepthBuffer(uint32_t width, uint32_t height, VkFormat format)
{
    VkExtent3D extent;
    extent.width = width;
    extent.height = height;
    extent.depth = 1;
    return createImage(extent, format, 1, 1, VK_SAMPLE_COUNT_1_BIT,
                       VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

Image Device::createImage(VkExtent3D const& extent, VkFormat format, uint32_t mip_levels, uint32_t array_layers,
                          VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage_flags)
{
    VkImageCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.imageType = ((extent.depth == 1) ?
                                ((extent.height == 1) ? VK_IMAGE_TYPE_1D : VK_IMAGE_TYPE_2D) : VK_IMAGE_TYPE_3D);
    create_info.format = format;
    create_info.extent = extent;
    create_info.mipLevels = mip_levels;
    create_info.arrayLayers = array_layers;
    create_info.samples = samples;
    create_info.tiling = tiling;
    create_info.usage = usage_flags;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;                  // only relevant for SHARING_MODE_CONCURRENT
    create_info.pQueueFamilyIndices = nullptr;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage image;
    VkResult res = vkCreateImage(m_device, &create_info, nullptr, &image);
    checkVulkanError(res, "Error in vkCreateImage.");
    return Image(m_device, image, extent, format);
}

Sampler Device::createSampler()
{
    return createSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 16.f);
}

inline Sampler Device::createSampler(VkFilter min_mag_filter, VkSamplerAddressMode address_mode, float max_anisotropy)
{
    return createSampler(min_mag_filter, min_mag_filter, VK_SAMPLER_MIPMAP_MODE_LINEAR,
                         address_mode, address_mode, address_mode,
                         0.f, max_anisotropy, 0.f, 0.f, VK_BORDER_COLOR_INT_OPAQUE_BLACK);
}

inline Sampler Device::createSampler(VkFilter min_filter, VkFilter mag_filter, VkSamplerMipmapMode mipmap_mode,
                                     VkSamplerAddressMode address_mode_u, VkSamplerAddressMode address_mode_v,
                                     VkSamplerAddressMode address_mode_w, float mip_lod_bias, float max_anisotropy,
                                     float min_lod, float max_lod, VkBorderColor border_color)
{
    VkSamplerCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.magFilter = min_filter;
    create_info.minFilter = mag_filter;
    create_info.mipmapMode = mipmap_mode;
    create_info.addressModeU = address_mode_u;
    create_info.addressModeV = address_mode_v;
    create_info.addressModeW = address_mode_w;
    create_info.mipLodBias = mip_lod_bias;
    create_info.anisotropyEnable = (max_anisotropy > 0.f) ? VK_TRUE : VK_FALSE;
    create_info.maxAnisotropy = max_anisotropy;
    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    create_info.minLod = min_lod;
    create_info.maxLod = max_lod;
    create_info.borderColor = border_color;
    create_info.unnormalizedCoordinates = VK_FALSE;
    VkSampler sampler;
    VkResult res = vkCreateSampler(m_device, &create_info, nullptr, &sampler);
    checkVulkanError(res, "Error in vkCreateSampler.");
    return Sampler(m_device, sampler);
}

DeviceMemory Device::allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags)
{
    return m_allocator.allocateMemory(requested_size, flags);
}

DeviceMemory Device::allocateMemory(VkMemoryRequirements const& requirements, VkMemoryPropertyFlags required_flags)
{
    return m_allocator.allocateMemory(requirements, required_flags);
}

CommandPool Device::createCommandPool(VkCommandPoolCreateFlags requested_flags, uint32_t queue_family_index)
{
    VkCommandPoolCreateInfo pool_create_info;
    pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = requested_flags;
    pool_create_info.queueFamilyIndex = queue_family_index;
    VkCommandPool command_pool;
    VkResult res = vkCreateCommandPool(m_device, &pool_create_info, nullptr, &command_pool);
    checkVulkanError(res, "Error in vkCreateCommandPool.");
    return CommandPool(m_device, command_pool, queue_family_index);
}

Queue Device::getQueue(uint32_t queue_family, uint32_t queue_index)
{
    VkQueue queue;
    vkGetDeviceQueue(m_device, queue_family, queue_index, &queue);      //@todo this will crash if family or index are invalid
    return Queue(queue);
}

ShaderModule Device::createShaderModule(SpirvCode const& code)
{
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.codeSize = code.getSize();
    create_info.pCode = code.getCode();
    VkShaderModule shader_module;
    VkResult res = vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module);
    checkVulkanError(res, "Error in vkCreateShaderModule.");
    return ShaderModule(m_device, shader_module);
}

std::vector<Framebuffer> Device::createFramebuffers(Swapchain& swapchain, RenderPass& render_pass)
{
    std::vector<GhulbusVulkan::ImageView> image_views = swapchain.createImageViews();
    std::vector<Framebuffer> framebuffers;
    framebuffers.reserve(image_views.size());
    for (std::size_t i = 0; i < image_views.size(); ++i) {
        VkImageView attachments[] = { image_views[i].getVkImageView() };
        VkFramebufferCreateInfo framebuffer_ci;
        framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_ci.pNext = nullptr;
        framebuffer_ci.flags = 0;
        framebuffer_ci.renderPass = render_pass.getVkRenderPass();
        framebuffer_ci.attachmentCount = 1;
        framebuffer_ci.pAttachments = attachments;
        framebuffer_ci.width = swapchain.getWidth();
        framebuffer_ci.height = swapchain.getHeight();
        framebuffer_ci.layers = 1;
        VkFramebuffer framebuffer;
        VkResult res = vkCreateFramebuffer(m_device, &framebuffer_ci, nullptr, &framebuffer);
        checkVulkanError(res, "Error in vkCreateFramebuffer.");
        framebuffers.emplace_back(m_device, std::move(image_views[i]), framebuffer);
    }
    return framebuffers;
}

std::vector<Framebuffer> Device::createFramebuffers(Swapchain& swapchain, RenderPass& render_pass,
                                                    ImageView& depth_stencil)
{
    std::vector<GhulbusVulkan::ImageView> image_views = swapchain.createImageViews();
    std::vector<Framebuffer> framebuffers;
    framebuffers.reserve(image_views.size());
    for (std::size_t i = 0; i < image_views.size(); ++i) {
        VkImageView attachments[] = { image_views[i].getVkImageView(), depth_stencil.getVkImageView() };
        VkFramebufferCreateInfo framebuffer_ci;
        framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_ci.pNext = nullptr;
        framebuffer_ci.flags = 0;
        framebuffer_ci.renderPass = render_pass.getVkRenderPass();
        framebuffer_ci.attachmentCount = 2;
        framebuffer_ci.pAttachments = attachments;
        framebuffer_ci.width = swapchain.getWidth();
        framebuffer_ci.height = swapchain.getHeight();
        framebuffer_ci.layers = 1;
        VkFramebuffer framebuffer;
        VkResult res = vkCreateFramebuffer(m_device, &framebuffer_ci, nullptr, &framebuffer);
        checkVulkanError(res, "Error in vkCreateFramebuffer.");
        framebuffers.emplace_back(m_device, std::move(image_views[i]), framebuffer);
    }
    return framebuffers;
}

RenderPassBuilder Device::createRenderPassBuilder()
{
    return RenderPassBuilder(m_device);
}

DescriptorSetLayoutBuilder Device::createDescriptorSetLayoutBuilder()
{
    return DescriptorSetLayoutBuilder(m_device);
}

DescriptorPoolBuilder Device::createDescriptorPoolBuilder()
{
    return DescriptorPoolBuilder(m_device);
}

PipelineLayoutBuilder Device::createPipelineLayoutBuilder()
{
    return PipelineLayoutBuilder(m_device);
}

PipelineBuilder Device::createGraphicsPipelineBuilder(uint32_t viewport_width, uint32_t viewport_height)
{
    return PipelineBuilder(m_device, viewport_width, viewport_height);
}

void Device::waitIdle()
{
    VkResult res = vkDeviceWaitIdle(m_device);
    checkVulkanError(res, "Error in vkDeviceWaitIdle.");
}
}
