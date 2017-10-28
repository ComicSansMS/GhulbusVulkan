#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_STRING_CONVERTERS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_STRING_CONVERTERS_HPP

/** @file
*
* @brief String Converters for Vulkan types.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <iosfwd>
#include <string>

namespace GHULBUS_VULKAN_NAMESPACE
{

char const* to_string(VkResult r);
char const* to_string(VkPhysicalDeviceType t);
char const* to_string(VkExtensionProperties const& ep);

std::string to_string(VkQueueFlags flags);

std::string memory_type_properties_to_string(uint32_t propertyFlags);

std::string memory_heap_properties_to_string(uint32_t propertyFlags);

std::string memory_size_to_string(VkDeviceSize size);

std::string version_to_string(uint32_t v);

std::string uuid_to_string(uint8_t uuid[VK_UUID_SIZE]);
}

std::ostream& operator<<(std::ostream& os, VkResult r);
std::ostream& operator<<(std::ostream& os, VkPhysicalDeviceType t);
std::ostream& operator<<(std::ostream& os, VkLayerProperties const& lp);
std::ostream& operator<<(std::ostream& os, VkExtensionProperties const& ep);

#endif
