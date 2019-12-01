#include <gbVk/DeviceBuilder.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <optional>

namespace GHULBUS_VULKAN_NAMESPACE
{
DeviceBuilder::DeviceBuilder(VkPhysicalDevice physical_device)
    :m_physicalDevice(physical_device)
{
}

void DeviceBuilder::addQueue(uint32_t queue_family, uint32_t n_queues)
{
    queue_create_infos.emplace_back();
    auto& queue_ci = queue_create_infos.back();
    queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.pNext = nullptr;
    queue_ci.flags = 0;    // reserved;
    queue_ci.queueFamilyIndex = queue_family;
    queue_ci.queueCount = n_queues;
    queue_create_priorities.emplace_back(n_queues, 1.0f);
    queue_ci.pQueuePriorities = queue_create_priorities.back().data();
}

void DeviceBuilder::addExtension(std::string extension)
{
    extensions.emplace_back(std::move(extension));
}

Device DeviceBuilder::create()
{
    GHULBUS_PRECONDITION(queue_create_infos.size() == queue_create_priorities.size());
    VkDeviceCreateInfo dev_create_info;
    dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_create_info.pNext = nullptr;
    dev_create_info.flags = 0;  // reserved

    dev_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    dev_create_info.pQueueCreateInfos = queue_create_infos.data();
    dev_create_info.enabledLayerCount = 0;
    dev_create_info.ppEnabledLayerNames = nullptr;
    std::vector<char const*> extensions_cstr;
    extensions_cstr.reserve(extensions.size());
    for (auto const& s : extensions) { extensions_cstr.push_back(s.c_str()); }
    dev_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions_cstr.size());
    dev_create_info.ppEnabledExtensionNames = extensions_cstr.data();

    VkPhysicalDeviceFeatures rf = requested_features.value_or(VkPhysicalDeviceFeatures{});
                                                                    // @todo: select requested features, compare
                                                                    //        against vkGetPhysicalDeviceFeatures()
    dev_create_info.pEnabledFeatures = &rf;     // this is used as an in/out param by vkCreateDevice

    VkDevice device;
    VkResult res = vkCreateDevice(m_physicalDevice, &dev_create_info, nullptr, &device);
    checkVulkanError(res, "Error in vkCreateDevice.");
    return Device(m_physicalDevice, device);
}
}
