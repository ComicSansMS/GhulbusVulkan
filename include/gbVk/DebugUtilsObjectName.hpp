#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEBUG_UTILS_OBJECT_NAME_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEBUG_UTILS_OBJECT_NAME_HPP

/** @file
*
* @brief Debug Utils Object Naming (requires VK_EXT_debug_utils).
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/
#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <bit>
#include <cstdint>
#include <concepts>

namespace GHULBUS_VULKAN_NAMESPACE::DebugUtils
{
void setObjectName(VkDevice device, char const* name, uint64_t object_handle, VkObjectType object_type);

template<typename Handle_T> requires(!std::same_as<Handle_T, std::uint64_t>)
void setObjectName(VkDevice device, char const* name, Handle_T object_handle, VkObjectType object_type)
{
    setObjectName(device, name, std::bit_cast<std::uint64_t>(object_handle), object_type);
}

}

#endif

