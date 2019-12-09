#include <gbGraphics/Window.hpp>

#include <gbBase/Assert.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#   define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include <limits>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
struct Window::GLFW_Pimpl {

};

Window::Window(int width, int height)
    :m_width(width), m_height(height), m_glfw(std::make_unique<GLFW_Pimpl>())
{
    GHULBUS_PRECONDITION(width > 0);
    GHULBUS_PRECONDITION(height > 0);
}

Window::~Window() = default;
}
