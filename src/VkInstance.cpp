#include <gbVk/VkInstance.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <limits>

namespace GHULBUS_VULKAN_NAMESPACE
{

std::vector<VkLayerProperties> VkInstance::enumerateInstanceLayerProperties()
{
    uint32_t layer_count = 0;
    VkResult res = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    if(res != VK_SUCCESS) { GHULBUS_THROW(Exceptions::VulkanError() << Exception_Info::vulkan_error_code(res),
                                          "Error in vkEnumerateInstanceLayerProperties."); }
    std::vector<VkLayerProperties> layer_props;
    layer_props.resize(layer_count);
    res = vkEnumerateInstanceLayerProperties(&layer_count, layer_props.data());
    if(res != VK_SUCCESS) { GHULBUS_THROW(Exceptions::VulkanError() << Exception_Info::vulkan_error_code(res),
                                          "Error in vkEnumerateInstanceLayerProperties."); }
    if(res != VK_SUCCESS) { GHULBUS_THROW(Exceptions::VulkanError() << Exception_Info::vulkan_error_code(res),
                                          "Error in vkEnumerateInstanceLayerProperties."); }
    GHULBUS_ASSERT_PRD(layer_count == layer_props.size());
    return layer_props;
}

std::vector<VkExtensionProperties> VkInstance::enumerateLayerExtensionProperties(VkLayerProperties const& layer)
{
    uint32_t extension_count = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(layer.layerName, &extension_count, nullptr);
    if(res != VK_SUCCESS) { GHULBUS_THROW(Exceptions::VulkanError() << Exception_Info::vulkan_error_code(res),
                                          "Error in vkEnumerateInstanceExtensionProperties."); }
    std::vector<VkExtensionProperties> extension_props;
    extension_props.resize(extension_count);
    res = vkEnumerateInstanceExtensionProperties(layer.layerName, &extension_count, extension_props.data());
    if(res != VK_SUCCESS) { GHULBUS_THROW(Exceptions::VulkanError() << Exception_Info::vulkan_error_code(res),
                                          "Error in vkEnumerateInstanceExtensionProperties."); }
    GHULBUS_ASSERT_PRD(extension_count == extension_props.size());
    return extension_props;
}

VkInstance VkInstance::createInstance()
{
    return createInstance(nullptr, Version(),
#ifdef NDEBUG
        Layers(),
#else
        Layers(Layers::ActivateValidationLayers()),
#endif
        std::vector<char const*>());
}

VkInstance VkInstance::createInstance(char const* application_name, Version const& application_version,
                                      Layers enabled_layers, std::vector<char const*> const& additional_layers)
{
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = application_version.version;
    app_info.pEngineName = "GhulbusVulkan";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    std::vector<char const*> requested_layers;
    if(enabled_layers.LUNARG_api_dump)             { requested_layers.push_back("VK_LAYER_LUNARG_api_dump"); }
    if(enabled_layers.LUNARG_monitor)              { requested_layers.push_back("VK_LAYER_LUNARG_monitor"); }
    if(enabled_layers.LUNARG_core_validation)      { requested_layers.push_back("VK_LAYER_LUNARG_core_validation"); }
    if(enabled_layers.LUNARG_object_tracker)       { requested_layers.push_back("VK_LAYER_LUNARG_object_tracker"); }
    if(enabled_layers.LUNARG_parameter_validation) { requested_layers.push_back("VK_LAYER_LUNARG_parameter_validation"); }
    if(enabled_layers.GOOGLE_threading)            { requested_layers.push_back("VK_LAYER_GOOGLE_threading"); }
    requested_layers.insert(end(requested_layers), begin(additional_layers), end(additional_layers));

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;  // reserved
    create_info.pApplicationInfo = &app_info;
    GHULBUS_PRECONDITION(requested_layers.size() < std::numeric_limits<uint32_t>::max());
    create_info.enabledLayerCount = static_cast<uint32_t>(requested_layers.size());
    create_info.ppEnabledLayerNames = requested_layers.data();
    create_info.enabledExtensionCount = 0;
    create_info.ppEnabledExtensionNames = nullptr;

    ::VkInstance instance;
    VkResult res = vkCreateInstance(&create_info, nullptr, &instance);
    if(res != VK_SUCCESS) { GHULBUS_THROW(Exceptions::VulkanError() << Exception_Info::vulkan_error_code(res),
                                          "Error in vkCreateInstance."); }
    return VkInstance(instance);
}

VkInstance::VkInstance(::VkInstance vk_instance)
    :m_instance(vk_instance)
{}

VkInstance::~VkInstance()
{
    if(m_instance) { vkDestroyInstance(m_instance, nullptr); }
}

VkInstance::VkInstance(VkInstance&& rhs)
    :m_instance(rhs.m_instance)
{
    rhs.m_instance = nullptr;
}

}
