#include <gbVk/Device.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/CommandPool.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/Swapchain.hpp>

#include <gbBase/Assert.hpp>

#include <utility>

namespace GHULBUS_VULKAN_NAMESPACE
{
Device::Device(VkPhysicalDevice physical_device, VkDevice logical_device)
    :m_device(logical_device), m_physicalDevice(physical_device)
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
    :m_device(rhs.m_device), m_physicalDevice(rhs.m_physicalDevice)
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

Swapchain Device::createSwapChain(VkSurfaceKHR surface, uint32_t queue_family)
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
    create_info.oldSwapchain = nullptr;

    VkSwapchainKHR swapchain;
    res = vkCreateSwapchainKHR(m_device, &create_info, nullptr, &swapchain);
    checkVulkanError(res, "Error in vkCreateSwapchainKHR.");

    return Swapchain(m_device, swapchain);
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

Buffer Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage_flags)
{
    VkBufferCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.size = size;
    create_info.usage = usage_flags;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    VkBuffer buffer;
    VkResult res = vkCreateBuffer(m_device, &create_info, nullptr, &buffer);
    checkVulkanError(res, "Error in vkCreateBuffer.");
    return Buffer(m_device, buffer);
}

Image Device::createImage()
{
    VkImageCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    create_info.extent.width = 1280;
    create_info.extent.height = 720;
    create_info.extent.depth = 1;
    create_info.mipLevels = 0;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = nullptr;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImage image;
    VkResult res = vkCreateImage(m_device, &create_info, nullptr, &image);
    checkVulkanError(res, "Error in vkCreateImage.");
    return Image(m_device, image);
}

DeviceMemory Device::allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags)
{
    auto const memory_type_index = PhysicalDevice(m_physicalDevice).findMemoryTypeIndex(flags);
    if(!memory_type_index) {
        GHULBUS_THROW(Exceptions::ProtocolViolation(), "No matching memory type available.");
    }
    VkMemoryAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.allocationSize = requested_size;
    alloc_info.memoryTypeIndex = *memory_type_index;

    VkDeviceMemory mem;
    VkResult res = vkAllocateMemory(m_device, &alloc_info, nullptr, &mem);
    checkVulkanError(res, "Error in vkAllocateMemory.");
    return DeviceMemory(m_device, mem);
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
    return CommandPool(m_device, command_pool);
}

VkQueue Device::getQueue(uint32_t queue_family, uint32_t queue_index)
{
    VkQueue queue;
    vkGetDeviceQueue(m_device, queue_family, queue_index, &queue);      //@todo this will crash if family or index are invalid
    return queue;
}

void Device::waitIdle()
{
    VkResult res = vkDeviceWaitIdle(m_device);
    checkVulkanError(res, "Error in vkDeviceWaitIdle.");
}
}
