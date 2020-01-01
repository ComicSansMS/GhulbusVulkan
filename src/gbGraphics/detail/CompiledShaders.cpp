#include <gbGraphics/detail/CompiledShaders.hpp>

namespace {
std::uint32_t g_draw2dVertex[] =
#   include <gbGraphics/detail/shader/draw2d.vert.h>
;

std::uint32_t g_draw2dFragment[] =
#   include <gbGraphics/detail/shader/draw2d.frag.h>
;
}

namespace GHULBUS_GRAPHICS_NAMESPACE::detail
{
namespace shader
{
ShaderData draw2dVertex()
{
    return ShaderData{ reinterpret_cast<std::byte const*>(g_draw2dVertex), sizeof(g_draw2dVertex) };
}

ShaderData draw2dFragment()
{
    return ShaderData{ reinterpret_cast<std::byte const*>(g_draw2dFragment), sizeof(g_draw2dFragment) };
}
}
}
