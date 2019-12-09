#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP

/** @file
*
* @brief Graphics Window.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <cstdint>
#include <memory>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class Window {
private:
    uint32_t m_width;
    uint32_t m_height;
    struct GLFW_Pimpl;
    std::unique_ptr<GLFW_Pimpl> m_glfw;
public:
    Window(int width, int height);
    ~Window();
};
}
#endif
