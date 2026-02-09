#include <gbVk/DebugUtilsMessenger.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Instance.hpp>

#include <gbBase/Assert.hpp>

#include <utility>

namespace GHULBUS_VULKAN_NAMESPACE
{
DebugUtilsMessenger::DebugUtilsMessenger(Instance& instance)
    :DebugUtilsMessenger(instance, allSeverities(), allTypes())
{
}

DebugUtilsMessenger::DebugUtilsMessenger(Instance& instance,
                                         VkDebugUtilsMessageSeverityFlagsEXT severity_flags,
                                         VkDebugUtilsMessageTypeFlagsEXT type_flags)
    :m_debugUtilsMessenger(nullptr), m_instance(instance.getVkInstance()),
     m_vkCreateDebugUtilsMessenger(nullptr), m_vkDestroyDebugUtilsMessenger(nullptr)
{
    m_vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));
    m_vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
    if ((!m_vkCreateDebugUtilsMessenger) || (!m_vkDestroyDebugUtilsMessenger)) {
        GHULBUS_THROW(Exceptions::VulkanError{} << Exception_Info::vulkan_error_code(VK_ERROR_EXTENSION_NOT_PRESENT),
                      "Extension VK_EXT_debug_utils was not loaded correctly.");
    }
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.messageSeverity = severity_flags;
    create_info.messageType = type_flags;
    create_info.pfnUserCallback = static_callback;
    create_info.pUserData = this;
    VkResult const res = m_vkCreateDebugUtilsMessenger(m_instance, &create_info, nullptr, &m_debugUtilsMessenger);
    checkVulkanError(res, "Error in vkCreateDebugUtilsMessengerEXT.");
}

DebugUtilsMessenger::~DebugUtilsMessenger()
{
    if (m_debugUtilsMessenger) {
        m_vkDestroyDebugUtilsMessenger(m_instance, m_debugUtilsMessenger, nullptr);
    }
}


DebugUtilsMessenger::DebugUtilsMessenger(DebugUtilsMessenger&& rhs) noexcept
    :m_debugUtilsMessenger(std::exchange(rhs.m_debugUtilsMessenger, nullptr)),
     m_instance(std::exchange(rhs.m_instance, nullptr)),
     m_userCallbacks(std::exchange(rhs.m_userCallbacks, std::vector<Callback>{})),
     m_vkCreateDebugUtilsMessenger(rhs.m_vkCreateDebugUtilsMessenger),
     m_vkDestroyDebugUtilsMessenger(rhs.m_vkDestroyDebugUtilsMessenger)
{}

void DebugUtilsMessenger::addCallback(Callback callback_function)
{
    m_userCallbacks.emplace_back(std::move(callback_function));
}

VkDebugUtilsMessageSeverityFlagsEXT DebugUtilsMessenger::allSeverities() {
    return VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
}

VkDebugUtilsMessageTypeFlagsEXT DebugUtilsMessenger::allTypes() {
    return VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
           VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
}

/* static */ char const* DebugUtilsMessenger::translateMessageTypeFlags(VkDebugUtilsMessageTypeFlagsEXT flags)
{
    switch (flags) {
    case 0: return "";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: return "General";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: return "Validation";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Performance";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        return "General Validation";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        return "General Performance";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        return "Validation Performance";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        return "General Validation Performance";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
        return "Device address bindings changed";
    }
    GHULBUS_UNREACHABLE_MESSAGE("Invalid Message Type");
}

/* static  */ char const* DebugUtilsMessenger::translateObjectType(VkObjectType object_type) {
    switch (object_type) {
    default: return "<Unkown Object Type>";
    case VK_OBJECT_TYPE_UNKNOWN: return "Unknown";
    case VK_OBJECT_TYPE_INSTANCE: return "Instance";
    case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return "PhysicalDevice";
    case VK_OBJECT_TYPE_DEVICE: return "Device";
    case VK_OBJECT_TYPE_QUEUE: return "Queue";
    case VK_OBJECT_TYPE_SEMAPHORE: return "Semaphore";
    case VK_OBJECT_TYPE_COMMAND_BUFFER: return "CommandBuffer";
    case VK_OBJECT_TYPE_FENCE: return "Fence";
    case VK_OBJECT_TYPE_DEVICE_MEMORY: return "DeviceMemory";
    case VK_OBJECT_TYPE_BUFFER: return "Buffer";
    case VK_OBJECT_TYPE_IMAGE: return "Image";
    case VK_OBJECT_TYPE_EVENT: return "Event";
    case VK_OBJECT_TYPE_QUERY_POOL: return "QueryPool";
    case VK_OBJECT_TYPE_BUFFER_VIEW: return "BufferView";
    case VK_OBJECT_TYPE_IMAGE_VIEW: return "ImageView";
    case VK_OBJECT_TYPE_SHADER_MODULE: return "ShaderModule";
    case VK_OBJECT_TYPE_PIPELINE_CACHE: return "PipelineCache";
    case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return "PipelineLayout";
    case VK_OBJECT_TYPE_RENDER_PASS: return "RenderPass";
    case VK_OBJECT_TYPE_PIPELINE: return "Pipeline";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "DescriptorSetLayout";
    case VK_OBJECT_TYPE_SAMPLER: return "Sampler";
    case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return "DescriptorPool";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET: return "DescriptorSet";
    case VK_OBJECT_TYPE_FRAMEBUFFER: return "Framebuffer";
    case VK_OBJECT_TYPE_COMMAND_POOL: return "CommandPool";
    case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return "UpdateTemplate";
    case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return "SamplerYcbrConversion";
    case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT: return "PrivateDataSlot";
    case VK_OBJECT_TYPE_SURFACE_KHR: return "Surface_KHR";
    case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return "Swapchain_KHR";
    case VK_OBJECT_TYPE_DISPLAY_KHR: return "Display_KHR";
    case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return "DisplayMode_KHR";
    }
}

/* static */ VkBool32 DebugUtilsMessenger::static_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                           VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
                                                           void* pUserData)
{
    DebugUtilsMessenger* thisptr = reinterpret_cast<DebugUtilsMessenger*>(pUserData);
    return thisptr->callback(messageSeverity, messageTypes, pCallbackData);
}

VkBool32 DebugUtilsMessenger::callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                       VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData)
{
    for (auto const& f : m_userCallbacks) {
        if (f(messageSeverity, messageTypes, pCallbackData) == Return::Abort) {
            return VK_TRUE;
        }
    }
    return VK_FALSE;

}

}
