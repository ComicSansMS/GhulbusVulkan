#include <gbGraphics/Window.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/UnusedVariable.hpp>
#include <gbBase/Log.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/SubmitStaging.hpp>
#include <gbVk/Queue.hpp>

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
    std::optional<VkExtent2D> resized_to;

    GLFW_Pimpl(GraphicsInstance& instance, uint32_t width, uint32_t height, char8_t const* window_title)
        :window(nullptr), surface(nullptr), graphics_instance(nullptr)
    {
        GHULBUS_PRECONDITION(width > 0);
        GHULBUS_PRECONDITION(height > 0);

        glfwWindowHint(GLFW_RESIZABLE, 1);
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

    static void static_resizeCallback(GLFWwindow* window, int width, int height)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->resizeCallback(width, height);
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

    void resizeCallback(int width, int height)
    {
        GHULBUS_LOG(Trace, "Resize: " << width << "x" << height);
        resized_to = VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    }
};

Window::Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title)
    :m_width(width), m_height(height), m_glfw(std::make_unique<GLFW_Pimpl>(instance, width, height, window_title)),
    m_swapchain(instance.getVulkanDevice().createSwapchain(m_glfw->surface, instance.getGraphicsQueueFamilyIndex())),
    m_presentCommandBuffers(m_glfw->graphics_instance->getCommandPoolRegistry().allocateCommandBuffersGraphics(m_swapchain.getNumberOfImages())),
    m_presentQueue(&m_glfw->graphics_instance->getGraphicsQueue()),
    m_presentFence(m_glfw->graphics_instance->getVulkanDevice().createFence())
{
    prepareBackbuffer();
}

Window::~Window()
{
    if (m_backBuffer) { m_backBuffer->fence.wait(); }
}

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

Window::PresentStatus Window::present()
{
    return present(m_backBuffer->semaphore);
}

Window::PresentStatus Window::present(GhulbusVulkan::Semaphore& semaphore)
{
    GHULBUS_PRECONDITION(m_backBuffer);
    m_backBuffer->fence.wait();

    uint32_t const idx = m_backBuffer->image.getSwapchainIndex();
    m_presentQueue->stageSubmission(std::move(m_windowSubmits));
    m_windowSubmits = GhulbusVulkan::SubmitStaging{};
    m_presentFence.reset();
    m_presentQueue->submitAllStaged(m_presentFence);
    try {
        m_swapchain.present(m_presentQueue->getVkQueue(), semaphore, std::move(m_backBuffer->image));
    } catch(GhulbusVulkan::Exceptions::VulkanError const& e) {
        VkResult const* const res = boost::get_error_info<GhulbusVulkan::Exception_Info::vulkan_error_code>(e);
        if(!res || ((*res != VK_ERROR_OUT_OF_DATE_KHR) && (*res != VK_SUBOPTIMAL_KHR))) { throw; }
        // we lost the back buffer; we'll have to recreate everything
        m_backBuffer.reset();
        return PresentStatus::InvalidBackbufferLostFrame;
    }

    m_presentFence.wait();
    if (m_glfw->resized_to) { m_backBuffer.reset(); return PresentStatus::InvalidBackbuffer; }
    prepareBackbuffer();
    if(!m_backBuffer) { return PresentStatus::InvalidBackbuffer; }
    return PresentStatus::Ok;
}

uint32_t Window::getNumberOfImagesInSwapchain() const
{
    return m_swapchain.getNumberOfImages();
}

uint32_t Window::getCurrentImageSwapchainIndex() const
{
    return m_backBuffer->image.getSwapchainIndex();
}

GhulbusVulkan::Semaphore& Window::getCurrentImageAcquireSemaphore()
{
    return m_backBuffer->semaphore;
}

GhulbusVulkan::Swapchain& Window::getSwapchain()
{
    return m_swapchain;
}

GhulbusVulkan::Swapchain::AcquiredImage& Window::getAcquiredImage()
{
    return m_backBuffer->image;
}

void Window::addRecreateSwapchainCallback(RecreateSwapchainCallback cb)
{
    GHULBUS_PRECONDITION(cb);
    m_recreateCallbacks.push_back(cb);
}

void Window::recreateSwapchain()
{
    GhulbusGraphics::GraphicsInstance& instance = *m_glfw->graphics_instance;
    GhulbusVulkan::Device& device = instance.getVulkanDevice();
    device.waitIdle();
    m_presentFence.reset();
    m_swapchain.recreate(device);
    prepareBackbuffer();
    m_glfw->resized_to = std::nullopt;
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
        try {
            m_backBuffer->image = m_swapchain.acquireNextImage(m_backBuffer->fence, m_backBuffer->semaphore);
        } catch(GhulbusVulkan::Exceptions::VulkanError const& e) {
            VkResult const* const res = boost::get_error_info<GhulbusVulkan::Exception_Info::vulkan_error_code>(e);
            if(!res || ((*res != VK_ERROR_OUT_OF_DATE_KHR) && (*res != VK_SUBOPTIMAL_KHR))) { throw; }
            if(*res == VK_SUBOPTIMAL_KHR) {
                // layout is suboptimal for presentation, but we can still finish the frame.
                // this will be fixed after present()
                return;
            } else {
                // we lost the back buffer; we'll have to recreate everything
                GHULBUS_ASSERT(*res == VK_ERROR_OUT_OF_DATE_KHR);
                m_backBuffer.reset();
                return;
            }
        }
    }
}
}
