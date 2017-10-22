#include <gbVk/Instance.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <limits>

namespace GHULBUS_VULKAN_NAMESPACE
{

std::vector<VkLayerProperties> Instance::enumerateInstanceLayerProperties()
{
    uint32_t layer_count = 0;
    VkResult res = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    checkVulkanError(res, "Error in vkEnumerateInstanceLayerProperties.");
    std::vector<VkLayerProperties> layer_props;
    layer_props.resize(layer_count);
    res = vkEnumerateInstanceLayerProperties(&layer_count, layer_props.data());
    checkVulkanError(res, "Error in vkEnumerateInstanceLayerProperties.");
    GHULBUS_ASSERT_PRD(layer_count == layer_props.size());
    return layer_props;
}

namespace {
std::vector<VkExtensionProperties> enumerateInstanceExtensionProperties_impl(char const* layer_name)
{
    uint32_t extension_count = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(layer_name, &extension_count, nullptr);
    checkVulkanError(res, "Error in vkEnumerateInstanceExtensionProperties.");
    std::vector<VkExtensionProperties> extension_props;
    extension_props.resize(extension_count);
    res = vkEnumerateInstanceExtensionProperties(layer_name, &extension_count, extension_props.data());
    checkVulkanError(res, "Error in vkEnumerateInstanceExtensionProperties.");
    GHULBUS_ASSERT_PRD(extension_count == extension_props.size());
    return extension_props;
}
}

std::vector<VkExtensionProperties> Instance::enumerateInstanceExtensionProperties()
{
    return enumerateInstanceExtensionProperties_impl(nullptr);
}

std::vector<VkExtensionProperties> Instance::enumerateInstanceExtensionProperties(VkLayerProperties const& layer)
{
    return enumerateInstanceExtensionProperties_impl(layer.layerName);
}

Instance Instance::createInstance()
{
    return createInstance(nullptr, Version(),
#ifdef NDEBUG
        Layers(),
#else
        Layers(Layers::ActivateValidationLayers()),
#endif
        Extensions());
}

Instance Instance::createInstance(char const* application_name, Version const& application_version,
                                  Layers const& enabled_layers, Extensions const& enabled_extensions)
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = application_version.version;
    app_info.pEngineName = "GhulbusVulkan";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;  // reserved
    create_info.pApplicationInfo = &app_info;
    std::vector<char const*> requested_layers = enabled_layers.getRequestedLayers();
    GHULBUS_PRECONDITION(requested_layers.size() < std::numeric_limits<uint32_t>::max());
    create_info.enabledLayerCount = static_cast<uint32_t>(requested_layers.size());
    create_info.ppEnabledLayerNames = requested_layers.data();
    std::vector<char const*> requested_extensions = enabled_extensions.getRequestedExtensions();
    GHULBUS_PRECONDITION(requested_extensions.size() < std::numeric_limits<uint32_t>::max());
    create_info.enabledExtensionCount = static_cast<uint32_t>(requested_extensions.size());
    create_info.ppEnabledExtensionNames = requested_extensions.data();

    VkInstance instance;
    VkResult res = vkCreateInstance(&create_info, nullptr, &instance);
    checkVulkanError(res, "Error in vkCreateInstance.");
    GHULBUS_ASSERT(instance);
    return Instance(instance);
}

Instance::Instance(VkInstance vk_instance)
    :m_instance(vk_instance)
{}

Instance::~Instance()
{
    if(m_instance) { vkDestroyInstance(m_instance, nullptr); }
}

Instance::Instance(Instance&& rhs)
    :m_instance(rhs.m_instance)
{
    rhs.m_instance = nullptr;
}

std::vector<VkPhysicalDevice> Instance::enumeratePhysicalDevices()
{
    uint32_t physdevcount = 0;
    VkResult res = vkEnumeratePhysicalDevices(m_instance, &physdevcount, 0);
    checkVulkanError(res, "Error in vkEnumeratePhysicalDevices.");
    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(physdevcount);
    res = vkEnumeratePhysicalDevices(m_instance, &physdevcount, physical_devices.data());
    checkVulkanError(res, "Error in vkEnumeratePhysicalDevices.");
    GHULBUS_ASSERT_PRD(physdevcount == physical_devices.size());
    return physical_devices;
}

}
