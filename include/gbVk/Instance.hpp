#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_INSTANCE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_INSTANCE_HPP

/** @file
*
* @brief Vulkan Instance.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <vector>

#if !defined VK_HEADER_VERSION or VK_HEADER_VERSION != GHULBUS_VULKAN_EXPECTED_VK_HEADER_VERSION
#   error Wrong Vulkan SDK version.
#endif

namespace GHULBUS_VULKAN_NAMESPACE
{

class Instance {
public:
    struct Version {
        uint32_t version;
        Version()
            : version(0)
        {}
        Version(uint16_t major, uint16_t minor, uint32_t patch)
            :version(VK_MAKE_VERSION(major, minor, patch))
        {}
    };
    struct Layers {
        struct ActivateValidationLayers {};
        bool LUNARG_api_dump;
        bool LUNARG_monitor;
        bool LUNARG_core_validation;
        bool LUNARG_object_tracker;
        bool LUNARG_parameter_validation;
        bool GOOGLE_threading;
        std::vector<char const*> additional_layers;
        Layers()
            :LUNARG_api_dump(false), LUNARG_monitor(false), LUNARG_core_validation(false),
             LUNARG_object_tracker(false), LUNARG_parameter_validation(false), GOOGLE_threading(false)
        {}

        Layers(ActivateValidationLayers)
            :LUNARG_api_dump(false), LUNARG_monitor(false), LUNARG_core_validation(true),
             LUNARG_object_tracker(true), LUNARG_parameter_validation(true), GOOGLE_threading(true)
        {}

        void addLayer(VkLayerProperties const& layer)
        {
            additional_layers.push_back(layer.layerName);
        }

        void addLayer(char const* layer_name)
        {
            additional_layers.push_back(layer_name);
        }

        std::vector<char const*> getRequestedLayers() const
        {
            std::vector<char const*> requested_layers;
            if(LUNARG_api_dump) { requested_layers.push_back("VK_LAYER_LUNARG_api_dump"); }
            if(LUNARG_monitor) { requested_layers.push_back("VK_LAYER_LUNARG_monitor"); }
            if(LUNARG_core_validation) { requested_layers.push_back("VK_LAYER_LUNARG_core_validation"); }
            if(LUNARG_object_tracker) { requested_layers.push_back("VK_LAYER_LUNARG_object_tracker"); }
            if(LUNARG_parameter_validation) { requested_layers.push_back("VK_LAYER_LUNARG_parameter_validation"); }
            if(GOOGLE_threading) { requested_layers.push_back("VK_LAYER_GOOGLE_threading"); }
            requested_layers.insert(end(requested_layers), begin(additional_layers), end(additional_layers));
            return requested_layers;
        }
    };

    struct Extensions {
        std::vector<char const*> additional_extensions;

        Extensions()
        {}

        void addExtension(VkExtensionProperties const& extension)
        {
            additional_extensions.push_back(extension.extensionName);
        }

        void addExtension(char const* extension_name)
        {
            additional_extensions.push_back(extension_name);
        }

        std::vector<char const*> getRequestedExtensions() const
        {
            return additional_extensions;
        }
    };

    static std::vector<VkLayerProperties> enumerateInstanceLayerProperties();
    static std::vector<VkExtensionProperties> enumerateInstanceExtensionProperties();
    static std::vector<VkExtensionProperties> enumerateInstanceExtensionProperties(VkLayerProperties const& layer);
    static Instance createInstance();
    static Instance createInstance(char const* application_name, Version const& application_version,
                                   Layers const& enabled_layers, Extensions const& enabled_extensions);

private:
    VkInstance m_instance;
public:

    Instance(VkInstance vk_instance);
    ~Instance();

    Instance(Instance const&) = delete;
    Instance& operator=(Instance const&) = delete;

    Instance(Instance&& rhs);
    Instance& operator=(Instance&&) = delete;

    std::vector<VkPhysicalDevice> enumeratePhysicalDevices();
};
}
#endif
