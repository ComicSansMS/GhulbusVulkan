#include <gbVk/DebugUtilsObjectName.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <utility>

namespace GHULBUS_VULKAN_NAMESPACE::DebugUtils
{
void setObjectName(VkDevice device, char const* name, uint64_t object_handle, VkObjectType object_type)
{
    PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectNameEXT =
        reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT"));
    if (!setDebugUtilsObjectNameEXT) {
        // debug utils extension was not loaded
        return;
    }
    VkDebugUtilsObjectNameInfoEXT name_info;
    name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    name_info.pNext = nullptr;
    name_info.objectType = object_type;
    name_info.objectHandle = object_handle;
    name_info.pObjectName = name;
    VkResult const res = setDebugUtilsObjectNameEXT(device, &name_info);
    checkVulkanError(res, "Error setting object name");
}
}
