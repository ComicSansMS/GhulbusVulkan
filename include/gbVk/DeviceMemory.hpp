#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_HPP

/** @file
*
* @brief Device Memory.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <gbVk/DeviceMemoryAllocator.hpp>

#include <cstddef>

namespace GHULBUS_VULKAN_NAMESPACE
{
using DeviceMemory = DeviceMemoryAllocator::DeviceMemory;
}
#endif
