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
class PhysicalDevice;

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
        bool LUNARG_standard_validation;
        std::vector<char const*> additional_layers;
        Layers()
            :LUNARG_standard_validation(false)
        {}

        Layers(ActivateValidationLayers)
            :LUNARG_standard_validation(true)
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
            if(LUNARG_standard_validation) { requested_layers.push_back("VK_LAYER_KHRONOS_validation"); }
            requested_layers.insert(end(requested_layers), begin(additional_layers), end(additional_layers));
            removeDuplicates(requested_layers);
            return requested_layers;
        }
    };

    struct Extensions {
        struct DeactivateSurfaceExtensions {};
        bool enable_surface_extension;
        bool enable_platform_specific_surface_extension;
        bool enable_debug_report_extension;
        bool enable_debug_utils_extension;
        std::vector<char const*> additional_extensions;

        Extensions()
            :enable_surface_extension(true), enable_platform_specific_surface_extension(true),
            enable_debug_report_extension(false)
        {}

        Extensions(DeactivateSurfaceExtensions)
            :enable_surface_extension(false), enable_platform_specific_surface_extension(false),
            enable_debug_report_extension(false)
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
            std::vector<char const*> requested_extensions;
            if (enable_surface_extension) { requested_extensions.push_back("VK_KHR_surface"); }
            if (enable_platform_specific_surface_extension) {
#               ifdef GHULBUS_CONFIG_VULKAN_PLATFORM_WIN32
                requested_extensions.push_back("VK_KHR_win32_surface");
#               endif
#               ifdef GHULBUS_CONFIG_VULKAN_PLATFORM_LINUX
                requested_extensions.push_back("VK_KHR_xcb_surface");
#               endif
            }
            if (enable_debug_report_extension) {
                // @todo deprecated by VK_EXT_debug_utils
                // https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_debug_report.html
                requested_extensions.push_back("VK_EXT_debug_report");
            }
            if (enable_debug_utils_extension) {
                // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VK_EXT_debug_utils
                requested_extensions.push_back("VK_EXT_debug_utils");
            }
            requested_extensions.insert(end(requested_extensions),
                                        begin(additional_extensions), end(additional_extensions));
            removeDuplicates(requested_extensions);
            return requested_extensions;
        }
    };

    static std::vector<VkLayerProperties> enumerateInstanceLayerProperties();
    static std::vector<VkExtensionProperties> enumerateInstanceExtensionProperties();
    static std::vector<VkExtensionProperties> enumerateInstanceExtensionProperties(VkLayerProperties const& layer);
    [[nodiscard]] static Instance createInstance();
    [[nodiscard]] static Instance createInstance(Extensions const& enabled_extensions);
    [[nodiscard]] static Instance createInstance(char const* application_name, Version const& application_version,
                                                 Layers const& enabled_layers, Extensions const& enabled_extensions);

private:
    static void removeDuplicates(std::vector<char const*>& v);

private:
    VkInstance m_instance;
public:
    explicit Instance(VkInstance vk_instance);
    ~Instance();

    Instance(Instance const&) = delete;
    Instance& operator=(Instance const&) = delete;

    Instance(Instance&& rhs);
    Instance& operator=(Instance&&) = delete;

    VkInstance getVkInstance();

    std::vector<PhysicalDevice> enumeratePhysicalDevices();

    static uint32_t getMaximumSupportedVulkanApiVersion();

    static uint32_t getVulkanApiVersion();
};
}
#endif
