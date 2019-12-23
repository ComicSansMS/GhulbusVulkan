#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_EXCEPTIONS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_EXCEPTIONS_HPP

/** @file
*
* @brief Exceptions.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbBase/Exception.hpp>

#include <ios>

namespace GHULBUS_GRAPHICS_NAMESPACE
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
        struct GHULBUS_GRAPHICS_API io_offset { };
    }
    /** Decorator record types.
     */
    namespace Records
    {
    }

    /** @name Decorators
     * @{
     */
    typedef boost::error_info<Tags::io_offset, std::streamsize> io_offset;
    /// @}
}
namespace Exceptions
{
    using namespace GHULBUS_BASE_NAMESPACE::Exceptions;

    struct GLFWError : public GHULBUS_BASE_NAMESPACE::Exceptions::impl::ExceptionImpl
    {};

    struct ShaderError : public GHULBUS_BASE_NAMESPACE::Exceptions::impl::ExceptionImpl
    {};
}
}
#endif
