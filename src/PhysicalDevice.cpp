#include <gbVk/PhysicalDevice.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/DeviceBuilder.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device)
    :m_physicalDevice(physical_device)
{
}

VkPhysicalDevice PhysicalDevice::getVkPhysicalDevice()
{
    return m_physicalDevice;
}

VkPhysicalDeviceProperties PhysicalDevice::getProperties()
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    return props;
}

VkPhysicalDeviceFeatures PhysicalDevice::getFeatures()
{
    VkPhysicalDeviceFeatures feat;
    vkGetPhysicalDeviceFeatures(m_physicalDevice, &feat);
    return feat;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::getMemoryProperties()
{
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &mem_props);
    return mem_props;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties()
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_props;
    queue_family_props.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queue_family_count, queue_family_props.data());
    GHULBUS_ASSERT_PRD(queue_family_count == queue_family_props.size());
    return queue_family_props;
}

VkFormatProperties PhysicalDevice::getFormatProperties(VkFormat format)
{
    VkFormatProperties ret;
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &ret);
    return ret;
}

std::optional<VkFormat> PhysicalDevice::findSupportedFormat(std::vector<VkFormat> const& candidates,
                                                            VkImageTiling tiling, VkFormatFeatureFlags features)
{
    auto matching_format = [this, tiling, features](VkFormat f) -> bool {
        VkFormatProperties const fmt_props = getFormatProperties(f);
        if (tiling == VK_IMAGE_TILING_LINEAR) {
            return (fmt_props.linearTilingFeatures & features) == features;
        } else {
            GHULBUS_ASSERT(tiling == VK_IMAGE_TILING_OPTIMAL);
            return (fmt_props.optimalTilingFeatures & features) == features;
        }
    };
    auto const ret = std::find_if(begin(candidates), end(candidates), matching_format);
    return (ret != end(candidates)) ? std::make_optional(*ret) : std::nullopt;
}

std::optional<VkFormat> PhysicalDevice::findDepthBufferFormat()
{
    std::vector<VkFormat>  const format_candidates{ VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT,
                                                    VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                                    VK_FORMAT_D16_UNORM };
    return findSupportedFormat(format_candidates, VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkImageFormatProperties PhysicalDevice::getImageFormatProperties(VkFormat format, VkImageType type,
    VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags create_flags)
{
    VkImageFormatProperties ret;
    VkResult res = vkGetPhysicalDeviceImageFormatProperties(m_physicalDevice, format, type, tiling, usage, create_flags, &ret);
    checkVulkanError(res, "Error in vkGetPhysicalDeviceImageFormatProperties.");
    return ret;
}

std::optional<uint32_t> PhysicalDevice::findMemoryTypeIndex(VkMemoryPropertyFlags requested_properties)
{
    auto const mem_props = getMemoryProperties();
    for(uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
        if((mem_props.memoryTypes[i].propertyFlags & requested_properties) == requested_properties) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<uint32_t> PhysicalDevice::findMemoryTypeIndex(VkMemoryPropertyFlags requested_properties,
                                                            VkMemoryRequirements const& requirements)
{
    auto mem_props = getMemoryProperties();
    for(std::uint32_t i = 0; i < mem_props.memoryTypeCount; ++i) {
        if(requirements.memoryTypeBits & (1 << i)) {
            auto const& candidate = mem_props.memoryTypes[i];
            if((candidate.propertyFlags & requested_properties) == requested_properties) {
                return i;
            }
        }
    }
    return std::nullopt;
}

bool PhysicalDevice::getSurfaceSupport(uint32_t queue_family, VkSurfaceKHR surface)
{
    VkBool32 ret;
    VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, queue_family, surface, &ret);
    checkVulkanError(res, "Error in vkGetPhysicalDeviceSurfaceSupportKHR.");
    return (ret == VK_TRUE) ? true : false;
}

std::vector<VkLayerProperties> PhysicalDevice::enumerateDeviceLayerProperties()
{
    uint32_t device_layer_count = 0;
    VkResult res = vkEnumerateDeviceLayerProperties(m_physicalDevice, &device_layer_count, nullptr);
    checkVulkanError(res, "Error in vkEnumerateDeviceLayerProperties.");
    std::vector<VkLayerProperties> device_layer_props;
    device_layer_props.resize(device_layer_count);
    res = vkEnumerateDeviceLayerProperties(m_physicalDevice, &device_layer_count, device_layer_props.data());
    checkVulkanError(res, "Error in vkEnumerateDeviceLayerProperties.");
    GHULBUS_ASSERT_PRD(device_layer_count == device_layer_props.size());
    return device_layer_props;
}

namespace {
std::vector<VkExtensionProperties> enumerateDeviceExtensionProperties_impl(VkPhysicalDevice physical_device, char const* layer_name)
{
    uint32_t device_extension_count = 0;
    VkResult res = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &device_extension_count, nullptr);
    checkVulkanError(res, "Error in vkEnumerateDeviceExtensionProperties.");
    std::vector<VkExtensionProperties> device_extension_props;
    device_extension_props.resize(device_extension_count);
    res = vkEnumerateDeviceExtensionProperties(physical_device, layer_name, &device_extension_count, device_extension_props.data());
    checkVulkanError(res, "Error in vkEnumerateDeviceExtensionProperties.");
    GHULBUS_ASSERT_PRD(device_extension_count == device_extension_props.size());
    return device_extension_props;
}
}

std::vector<VkExtensionProperties> PhysicalDevice::enumerateDeviceExtensionProperties()
{
    return enumerateDeviceExtensionProperties_impl(m_physicalDevice, nullptr);
}

std::vector<VkExtensionProperties> PhysicalDevice::enumerateDeviceExtensionProperties(VkLayerProperties layer)
{
    return enumerateDeviceExtensionProperties_impl(m_physicalDevice, layer.layerName);
}

namespace {
std::optional<uint32_t> determineDefaultQueueFamily(PhysicalDevice& pd)
{
    auto const queue_props = pd.getQueueFamilyProperties();
    bool candidate_found = false;
    uint32_t candidate_index = 0;
    for(auto const& qf : queue_props) {
        if(((qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) &&
           ((qf.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) &&
           ((qf.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0))
        {
#ifdef GHULBUS_CONFIG_VULKAN_PLATFORM_WIN32
            if(vkGetPhysicalDeviceWin32PresentationSupportKHR(pd.getVkPhysicalDevice(), candidate_index))
#endif
            {
                candidate_found = true;
                break;
            }
        }
        ++candidate_index;
    }

    return (candidate_found) ? std::optional<uint32_t>(candidate_index) : std::nullopt;
}
}

DeviceBuilder PhysicalDevice::createDeviceBuilder(NoSwapchainSupport_T const&)
{
    DeviceBuilder ret(m_physicalDevice);
    return ret;
}

DeviceBuilder PhysicalDevice::createDeviceBuilder()
{
    auto device_builder = createDeviceBuilder(no_swapchain_support);
    device_builder.addExtension("VK_KHR_swapchain");
    return device_builder;
}

Device PhysicalDevice::createDevice()
{
    auto device_builder = createDeviceBuilder();

    auto queue_family = determineDefaultQueueFamily(*this);
    if(!queue_family) {
        GHULBUS_THROW(Ghulbus::Exceptions::ProtocolViolation(), "Cannot find a suitable queue family on device.");
    }
    device_builder.addQueues(*queue_family, 1);

    return device_builder.create();
}

}
