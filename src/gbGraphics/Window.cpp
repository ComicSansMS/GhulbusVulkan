#include <gbGraphics/Window.hpp>

#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/Instance.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#   define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include <limits>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
struct Window::GLFW_Pimpl {
    GLFWwindow* window;
    VkSurfaceKHR surface;
    GhulbusGraphics::GraphicsInstance* graphics_instance;

    GLFW_Pimpl()
        :window(nullptr), surface(nullptr), graphics_instance(nullptr)
    {}

    ~GLFW_Pimpl()
    {
        if (surface) {
            vkDestroySurfaceKHR(graphics_instance->getVulkanInstance().getVkInstance(), surface, nullptr);
        }
        if (window) {
            glfwDestroyWindow(window);
        }
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        GHULBUS_UNUSED_VARIABLE(scancode);
        GHULBUS_UNUSED_VARIABLE(action);
        GHULBUS_UNUSED_VARIABLE(mods);
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
    }
};

Window::Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title)
    :m_width(width), m_height(height), m_glfw(std::make_unique<GLFW_Pimpl>())
{
    GHULBUS_PRECONDITION(width > 0);
    GHULBUS_PRECONDITION(height > 0);

    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_glfw->window =
        glfwCreateWindow(m_width, m_height, reinterpret_cast<char const*>(window_title), nullptr, nullptr);
    if (!m_glfw->window) {
        GHULBUS_THROW(Exceptions::GLFWError{}, "Unable to create GLFW window.");
    }

    m_glfw->graphics_instance = &instance;
    VkResult const res = glfwCreateWindowSurface(instance.getVulkanInstance().getVkInstance(),
        m_glfw->window, nullptr, &m_glfw->surface);
    if (res != VK_SUCCESS) {
        GHULBUS_THROW(Exceptions::GLFWError{}, "Unable to initialize Vulkan context.");
    }

    glfwSetWindowUserPointer(m_glfw->window, m_glfw.get());
    glfwSetKeyCallback(m_glfw->window, m_glfw->keyCallback);
}

Window::~Window() = default;

bool Window::isClosed()
{
    return glfwWindowShouldClose(m_glfw->window);
}

VkSurfaceKHR Window::getSurface()
{
    return m_glfw->surface;
}
}
