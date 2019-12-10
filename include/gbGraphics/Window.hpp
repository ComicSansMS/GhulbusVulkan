#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP

/** @file
*
* @brief Graphics Window.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

/// @todo: remove
#include <gbVk/config.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;

class Window {
private:
    uint32_t m_width;
    uint32_t m_height;
    struct GLFW_Pimpl;
    std::unique_ptr<GLFW_Pimpl> m_glfw;
public:
    Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title);
    ~Window();

    bool isClosed();

    /// @todo: remove
    VkSurfaceKHR getSurface();
};
}
#endif
