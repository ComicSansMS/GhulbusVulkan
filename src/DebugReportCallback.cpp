#include <gbVk/DebugReportCallback.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/Instance.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DebugReportCallback::DebugReportCallback(Instance& instance, VkDebugReportFlagsEXT flags)
    :m_debugReportCallback(nullptr), m_instance(instance.getVkInstance())
{
    m_vkCreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));
    m_vkDestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
        vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT"));
    if ((!m_vkCreateDebugReportCallback) || (!m_vkDestroyDebugReportCallback)) {
        GHULBUS_THROW(Exceptions::VulkanError{} << Exception_Info::vulkan_error_code(VK_ERROR_EXTENSION_NOT_PRESENT),
                      "Extension VK_EXT_debug_report was not loaded correctly.");
    }

    VkDebugReportCallbackCreateInfoEXT create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.pfnCallback = static_callback;
    create_info.pUserData = this;
    VkResult const res = m_vkCreateDebugReportCallback(m_instance, &create_info, nullptr, &m_debugReportCallback);
    checkVulkanError(res, "Error in vkCreateDebugReportCallbackEXT.");
}

DebugReportCallback::~DebugReportCallback()
{
    if (m_debugReportCallback) {
        m_vkDestroyDebugReportCallback(m_instance, m_debugReportCallback, nullptr);
    }
}

DebugReportCallback::DebugReportCallback(DebugReportCallback&& rhs)
    :m_debugReportCallback(rhs.m_debugReportCallback), m_instance(rhs.m_instance)
{
    rhs.m_debugReportCallback = nullptr;
}

void DebugReportCallback::addCallback(Callback callback_function)
{
    m_userCallbacks.emplace_back(std::move(callback_function));
}

VkBool32 DebugReportCallback::callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
                                       uint64_t object, size_t location, int32_t message_code,
                                       const char* layer_prefix, const char* message)
{
    for (auto const& f : m_userCallbacks) {
        if (f(flags, object_type, object, location, message_code, layer_prefix, message) == Return::Abort) {
            return VK_TRUE;
        }
    }
    return VK_FALSE;
}

VkBool32 DebugReportCallback::static_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
                                              uint64_t object, size_t location, int32_t message_code,
                                              const char* layer_prefix, const char* message, void* user_data)
{
    DebugReportCallback* thisptr = reinterpret_cast<DebugReportCallback*>(user_data);
    return thisptr->callback(flags, object_type, object, location, message_code, layer_prefix, message);
}

char const* DebugReportCallback::translateFlags(VkDebugReportFlagsEXT flags)
{
    switch (flags) {
    case VK_DEBUG_REPORT_INFORMATION_BIT_EXT: return "Information";
    case VK_DEBUG_REPORT_WARNING_BIT_EXT: return "Warning";
    case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT: return "Performance";
    case VK_DEBUG_REPORT_ERROR_BIT_EXT: return "Error";
    case VK_DEBUG_REPORT_DEBUG_BIT_EXT: return "Debug";
    default: return "";
    }
}


char const* DebugReportCallback::translateObjectType(VkDebugReportObjectTypeEXT object_type)
{
    switch (object_type) {
    default:
    case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT: return "Unknown";
    case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT: return "Instance";
    case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT: return "PhysicalDevice";
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT: return "Device";
    case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT: return "Queue";
    case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT: return "Semaphore";
    case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT: return "CommandBuffer";
    case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT: return "Fence";
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT: return "DeviceMemory";
    case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT: return "Buffer";
    case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT: return "Image";
    case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT: return "Event";
    case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT: return "QueryPool";
    case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT: return "BufferView";
    case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT: return "ImageView";
    case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT: return "ShaderModule";
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT: return "PipelineCache";
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT: return "PipelineLayout";
    case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT: return "RenderPass";
    case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT: return "Pipeline";
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT: return "DescriptorSetLayout";
    case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT: return "Sampler";
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT: return "DescriptorPool";
    case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT: return "DescriptorSet";
    case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT: return "Framebuffer";
    case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT: return "CommandPool";
    case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT: return "Surface";
    case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT: return "Swapchain";
    case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT: return "DebugReport";
    }
}
}
