#include <gbVk/PhysicalDevice.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <optional>

namespace GHULBUS_VULKAN_NAMESPACE
{

PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device)
    :m_physical_device(physical_device)
{
}

VkPhysicalDeviceProperties PhysicalDevice::getProperties()
{
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physical_device, &props);
    return props;
}

VkPhysicalDeviceFeatures PhysicalDevice::getFeatures()
{
    VkPhysicalDeviceFeatures feat;
    vkGetPhysicalDeviceFeatures(m_physical_device, &feat);
    return feat;
}

VkPhysicalDeviceMemoryProperties PhysicalDevice::getMemoryProperties()
{
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_props);
    return mem_props;
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::getQueueFamilyProperties()
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_family_props;
    queue_family_props.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_family_props.data());
    GHULBUS_ASSERT_PRD(queue_family_count == queue_family_props.size());
    return queue_family_props;
}

std::vector<VkLayerProperties> PhysicalDevice::enumerateDeviceLayerProperties()
{
    uint32_t device_layer_count = 0;
    VkResult res = vkEnumerateDeviceLayerProperties(m_physical_device, &device_layer_count, nullptr);
    checkVulkanError(res, "Error in vkEnumerateDeviceLayerProperties.");
    std::vector<VkLayerProperties> device_layer_props;
    device_layer_props.resize(device_layer_count);
    res = vkEnumerateDeviceLayerProperties(m_physical_device, &device_layer_count, device_layer_props.data());
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
    return enumerateDeviceExtensionProperties_impl(m_physical_device, nullptr);
}

std::vector<VkExtensionProperties> PhysicalDevice::enumerateDeviceExtensionProperties(VkLayerProperties layer)
{
    return enumerateDeviceExtensionProperties_impl(m_physical_device, layer.layerName);
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
            candidate_found = true;
            break;
        }
        ++candidate_index;
    }

    return (candidate_found) ? std::optional<uint32_t>(candidate_index) : std::nullopt;
}
}

Device PhysicalDevice::createDevice()
{
    auto queue_family = determineDefaultQueueFamily(*this);
    if(!queue_family) {
        GHULBUS_THROW(Ghulbus::Exceptions::ProtocolViolation(), "Cannot find a suitable queue family on device.");
    }

    VkDeviceCreateInfo dev_create_info;
    dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_create_info.pNext = nullptr;
    dev_create_info.flags = 0;  // reserved

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.resize(1);
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].pNext = nullptr;
    queue_create_infos[0].flags = 0;    // reserved;
    queue_create_infos[0].queueFamilyIndex = *queue_family;
    queue_create_infos[0].queueCount = 1;
    float device_queue_priorities[] = { 1.0f };
    queue_create_infos[0].pQueuePriorities = device_queue_priorities;

    dev_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    dev_create_info.pQueueCreateInfos = queue_create_infos.data();
    dev_create_info.enabledLayerCount = 0;
    dev_create_info.ppEnabledLayerNames = nullptr;
    dev_create_info.enabledExtensionCount = 0;
    dev_create_info.ppEnabledExtensionNames = nullptr;

    VkPhysicalDeviceFeatures physical_device_supported_features;
    vkGetPhysicalDeviceFeatures(m_physical_device, &physical_device_supported_features);
    VkPhysicalDeviceFeatures requested_features = {};           // @todo: select requested features
    dev_create_info.pEnabledFeatures = &requested_features;     // this is used as an in/out param by vkCreateDevice

    VkDevice device;
    VkResult res = vkCreateDevice(m_physical_device, &dev_create_info, nullptr, &device);
    checkVulkanError(res, "Error in vkCreateDevice.");
    return Device(device);
}

}
