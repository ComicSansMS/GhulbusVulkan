#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEBUG_REPORT_CALLBACK_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEBUG_REPORT_CALLBACK_HPP

/** @file
*
* @brief Debug Report Callback (requires VK_EXT_debug_report).
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

class DebugReportCallback {
public:
    enum class Return {
        Continue,
        Abort
    };
    using Callback = std::function<Return(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
                                          uint64_t object, size_t location, int32_t message_code,
                                          const char* layer_prefix, const char* message)>;
private:
    VkDebugReportCallbackEXT m_debugReportCallback;
    VkInstance m_instance;
    std::vector<Callback> m_userCallbacks;
    PFN_vkCreateDebugReportCallbackEXT m_vkCreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT m_vkDestroyDebugReportCallback;
public:
    explicit DebugReportCallback(Instance& instance, VkDebugReportFlagsEXT flags);
    ~DebugReportCallback();

    DebugReportCallback(DebugReportCallback const&) = delete;
    DebugReportCallback& operator=(DebugReportCallback const&) = delete;

    DebugReportCallback(DebugReportCallback&& rhs);
    DebugReportCallback& operator=(DebugReportCallback&&) = delete;

    void addCallback(Callback callback_function);

    static char const* translateFlags(VkDebugReportFlagsEXT flags);

    static char const* translateObjectType(VkDebugReportObjectTypeEXT object_type);
private:
    static VkBool32 static_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
                                    uint64_t object, size_t location, int32_t message_code,
                                    const char* layer_prefix, const char* message, void* user_data);

    VkBool32 callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
                      uint64_t object, size_t location, int32_t message_code,
                      const char* layer_prefix, const char* message);
};
}
#endif
