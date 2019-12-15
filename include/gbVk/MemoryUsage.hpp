#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_MEMORY_USAGE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_MEMORY_USAGE_HPP

/** @file
*
* @brief Memory Usage Flags.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
enum class MemoryUsage {
    GpuOnly,
    CpuOnly,
    CpuToGpu,
    GpuToCpu
};
}
#endif
