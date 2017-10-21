#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_VK_INSTANCE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_VK_INSTANCE_HPP

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
class VkInstance {
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
        Layers()
            :LUNARG_api_dump(false), LUNARG_monitor(false), LUNARG_core_validation(false),
             LUNARG_object_tracker(false), LUNARG_parameter_validation(false), GOOGLE_threading(false)
        {}

        Layers(ActivateValidationLayers)
            :LUNARG_api_dump(false), LUNARG_monitor(false), LUNARG_core_validation(true),
             LUNARG_object_tracker(true), LUNARG_parameter_validation(true), GOOGLE_threading(true)
        {}
    };
    static std::vector<VkLayerProperties> enumerateInstanceLayerProperties();
    static std::vector<VkExtensionProperties> enumerateLayerExtensionProperties(VkLayerProperties const& layer);
    static VkInstance createInstance();
    static VkInstance createInstance(char const* application_name, Version const& application_version,
                                     Layers enabled_layers, std::vector<char const*> const& additional_layers);

private:
    ::VkInstance m_instance;
public:

    VkInstance(::VkInstance vk_instance);
    ~VkInstance();

    VkInstance(VkInstance const&) = delete;
    VkInstance& operator=(VkInstance const&) = delete;

    VkInstance(VkInstance&& rhs);
    VkInstance& operator=(VkInstance&&) = delete;
};
}
#endif
