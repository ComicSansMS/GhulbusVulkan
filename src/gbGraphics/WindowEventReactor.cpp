#include <gbGraphics/WindowEventReactor.hpp>

#include <gbGraphics/Window.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

WindowEventReactor::WindowEventReactor()
{
}

void WindowEventReactor::onKey(Event::Key const& e)
{
    eventHandlers.keyEvent.invokeHandlers(e);
}

void WindowEventReactor::onText(Event::Text const& e)
{
    eventHandlers.textEvent.invokeHandlers(e);
}

void WindowEventReactor::onMouseMove(Event::MouseMove const& e)
{
    eventHandlers.mouseMoveEvent.invokeHandlers(e);
}

void WindowEventReactor::onMouseEnter(Event::MouseEnter const& e)
{
    eventHandlers.mouseEnterEvent.invokeHandlers(e);
}

void WindowEventReactor::onMouseLeave(Event::MouseLeave const& e)
{
    eventHandlers.mouseLeaveEvent.invokeHandlers(e);
}

void WindowEventReactor::onMouseClick(Event::MouseClick const& e)
{
    eventHandlers.mouseClickEvent.invokeHandlers(e);
}

void WindowEventReactor::onMouseScroll(Event::MouseScroll const& e)
{
    eventHandlers.mouseScrollEvent.invokeHandlers(e);
}

void WindowEventReactor::onViewportResize(Event::ViewportResize const& e)
{
    eventHandlers.viewportResizeEvent.invokeHandlers(e);
}
}
