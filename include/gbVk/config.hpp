#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_GBVK_CONFIG_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_GBVK_CONFIG_HPP

/** @file
 *
 * @brief General configuration for Vulkan.
 * @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
 */

#include <gbVk/gbVk_Export.hpp>

/** Specifies the API for a function declaration.
 * When building as a dynamic library, this is used to mark the functions that will be exported by the library.
 */
#define GHULBUS_VULKAN_API GHULBUS_LIBRARY_GBVK_EXPORT

/** \namespace GhulbusVulkan Namespace for the GhulbusVulkan library.
 * The implementation internally always uses this macro `GHULBUS_VULKAN_NAMESPACE` to refer to the namespace.
 * When building GhulbusVulkan yourself, you can globally redefine this macro to move to a different namespace.
 */
#ifndef GHULBUS_VULKAN_NAMESPACE
#   define GHULBUS_VULKAN_NAMESPACE GhulbusVulkan
#endif

#define GHULBUS_VULKAN_EXPECTED_VK_HEADER_VERSION 61

#endif
