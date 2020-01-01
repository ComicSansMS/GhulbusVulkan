#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DRAW_2D_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DRAW_2D_HPP

/** @file
*
* @brief Draw 2D.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/Program.hpp>
#include <gbGraphics/Renderer.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class Window;

class Draw2d {
private:
    Program m_program;
    Renderer m_renderer;
    Window* m_window;
public:
    Draw2d(GraphicsInstance& instance, Window& target_window);

    void draw();

    Renderer& getRenderer();
};
}
#endif

