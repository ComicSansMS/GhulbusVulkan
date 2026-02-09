#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEBUG_UTILS_MESSENGER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEBUG_UTILS_MESSENGER_HPP

/** @file
*
* @brief Debug Utils Messenger (requires VK_EXT_debug_utils).
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <functional>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Instance;

class DebugUtilsMessenger{
public:
    enum class Return {
        Continue,
        Abort
    };
    using Callback = std::function<Return(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                          VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                          VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData)>;
private:
    VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
    VkInstance m_instance;
    std::vector<Callback> m_userCallbacks;
    PFN_vkCreateDebugUtilsMessengerEXT m_vkCreateDebugUtilsMessenger;
    PFN_vkDestroyDebugUtilsMessengerEXT m_vkDestroyDebugUtilsMessenger;
public:
    explicit DebugUtilsMessenger(Instance& instance);
    explicit DebugUtilsMessenger(Instance& instance,
                                 VkDebugUtilsMessageSeverityFlagsEXT severity_flags,
                                 VkDebugUtilsMessageTypeFlagsEXT type_flags);
    ~DebugUtilsMessenger();

    DebugUtilsMessenger(DebugUtilsMessenger const&) = delete;
    DebugUtilsMessenger& operator=(DebugUtilsMessenger const&) = delete;

    DebugUtilsMessenger(DebugUtilsMessenger&& rhs) noexcept;
    DebugUtilsMessenger& operator=(DebugUtilsMessenger&&) = delete;

    void addCallback(Callback callback_function);

    static VkDebugUtilsMessageSeverityFlagsEXT allSeverities();

    static VkDebugUtilsMessageTypeFlagsEXT allTypes();

    static char const* translateMessageTypeFlags(VkDebugUtilsMessageTypeFlagsEXT flags);

    static char const* translateObjectType(VkObjectType object_type);
private:
    static VkBool32 static_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                    void* pUserData);

    VkBool32 callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                      VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData);
};


}

#endif

