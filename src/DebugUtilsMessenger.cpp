#include <gbVk/DebugUtilsMessenger.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Instance.hpp>

#include <gbBase/Assert.hpp>

#include <utility>

namespace GHULBUS_VULKAN_NAMESPACE
{
namespace {

}

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
