#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_COMPILED_SHADERS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_COMPILED_SHADERS_HPP

/** @file
*
* @brief Compiled Spirv Shaders.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <cstddef>
#include <cstdint>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
namespace detail
{
namespace shader
{
struct ShaderData {
    std::byte const* data;
    uint32_t size;
};

ShaderData draw2dVertex();
ShaderData draw2dFragment();
}
}
}
#endif
