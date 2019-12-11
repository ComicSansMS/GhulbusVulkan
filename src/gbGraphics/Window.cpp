#include <gbGraphics/Window.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
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

    GLFW_Pimpl(GraphicsInstance& instance, uint32_t width, uint32_t height, char8_t const* window_title)
        :window(nullptr), surface(nullptr), graphics_instance(nullptr)
    {
        GHULBUS_PRECONDITION(width > 0);
        GHULBUS_PRECONDITION(height > 0);

        glfwWindowHint(GLFW_RESIZABLE, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(width, height, reinterpret_cast<char const*>(window_title), nullptr, nullptr);
        if (!window) {
            GHULBUS_THROW(Exceptions::GLFWError{}, "Unable to create GLFW window.");
        }

        graphics_instance = &instance;
        VkInstance const vk_instance = instance.getVulkanInstance().getVkInstance();
        VkResult const res = glfwCreateWindowSurface(vk_instance, window, nullptr, &surface);
        if (res != VK_SUCCESS) {
            GHULBUS_THROW(Exceptions::GLFWError{}, "Unable to initialize Vulkan context.");
        }

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, GLFW_Pimpl::static_keyCallback);
    }

    ~GLFW_Pimpl()
    {
        if (surface) {
            vkDestroySurfaceKHR(graphics_instance->getVulkanInstance().getVkInstance(), surface, nullptr);
        }
        if (window) {
            glfwDestroyWindow(window);
        }
    }

    static void static_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->keyCallback(key, scancode, action, mods);
    }

    void keyCallback(int key, int scancode, int action, int mods)
    {
        GHULBUS_UNUSED_VARIABLE(scancode);
        GHULBUS_UNUSED_VARIABLE(action);
        GHULBUS_UNUSED_VARIABLE(mods);
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
    }
};

Window::Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title)
    :m_width(width), m_height(height), m_glfw(std::make_unique<GLFW_Pimpl>(instance, width, height, window_title)),
     m_swapchain(instance.getVulkanDevice().createSwapchain(m_glfw->surface, instance.getGraphicsQueueFamilyIndex())),
     m_presentCommandBuffers(m_glfw->graphics_instance->getCommandPoolRegistry().allocateGraphicCommandBuffers(m_swapchain.getNumberOfImages())),
     m_presentQueue(m_glfw->graphics_instance->getGraphicsQueue())
{
    prepareBackbuffer();
}

Window::~Window() = default;

bool Window::isDone()
{
    return glfwWindowShouldClose(m_glfw->window);
}

uint32_t Window::getWidth() const
{
    return m_width;
}

uint32_t Window::getHeight() const
{
    return m_height;
}

void Window::present()
{
    GHULBUS_PRECONDITION(m_backBuffer);
    auto const idx = m_backBuffer->image.getSwapchainIndex();
    auto command_buffer = m_presentCommandBuffers.getCommandBuffer(idx);
    // raw present (no renderpath)
    // we need to transition manually here (otherwise this is done by the renderpath)
    m_backBuffer->image->transition(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                    VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);


}

GhulbusVulkan::Swapchain& Window::getSwapchain()
{
    return m_swapchain;
}

void Window::prepareBackbuffer()
{
    if (!m_backBuffer) {
        GhulbusVulkan::Fence f = m_glfw->graphics_instance->getVulkanDevice().createFence();
        GhulbusVulkan::Semaphore s = m_glfw->graphics_instance->getVulkanDevice().createSemaphore();
        GhulbusVulkan::Swapchain::AcquiredImage image = m_swapchain.acquireNextImage(f, s);
        m_backBuffer.emplace(Backbuffer{ std::move(image), std::move(f), std::move(s) });
    } else {
        m_backBuffer->fence.reset();
        m_backBuffer->image = m_swapchain.acquireNextImage(m_backBuffer->fence, m_backBuffer->semaphore);
    }
}
}
