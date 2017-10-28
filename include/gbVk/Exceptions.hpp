#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_EXCEPTIONS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_EXCEPTIONS_HPP

/** @file
*
* @brief Exceptions.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>
#include <gbVk/StringConverters.hpp>

#include <vulkan/vulkan.h>

#include <gbBase/Exception.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
/** Exception decorators.
 */
namespace Exception_Info {
    using namespace GHULBUS_BASE_NAMESPACE::Exception_Info;

    /** Decorator Tags.
     * Tags are empty types used to uniquely identify a decoratory type.
     */
    namespace Tags
    {
        struct GHULBUS_VULKAN_API vulkan_error_code { };
    }
    /** Decorator record types.
     */
    namespace Records
    {
    }

    /** @name Decorators
     * @{
     */
    /** An int representation of the invalid enum value.
     */
    typedef boost::error_info<Tags::vulkan_error_code, VkResult> vulkan_error_code;
    /// @}
}
namespace Exceptions
{
    using namespace GHULBUS_BASE_NAMESPACE::Exceptions;

    struct VulkanError : public GHULBUS_BASE_NAMESPACE::Exceptions::impl::ExceptionImpl
    {};
}

inline void checkVulkanError(VkResult res, char const* error_msg)
{
    if(res != VK_SUCCESS) {
        GHULBUS_THROW(Exceptions::VulkanError() << Exception_Info::vulkan_error_code(res), error_msg);
    }
}
}
#endif
