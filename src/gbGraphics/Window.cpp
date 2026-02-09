#include <gbGraphics/Window.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/WindowEventReactor.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/UnusedVariable.hpp>

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

#include <format>
#include <limits>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
struct Window::GLFW_Pimpl {
    GLFWwindow* window;
    VkSurfaceKHR surface;
    GraphicsInstance* graphics_instance;
    WindowEventReactor event_reactor;
    Window* graphics_window;
    std::optional<VkExtent2D> resized_to;
    WindowEventReactor::KeyHandlerGuard default_key_handler_guard;
    WindowEventReactor::ViewportResizeGuard default_resize_handler_guard;

    GLFW_Pimpl(GraphicsInstance& instance, Window& ngraphics_window, uint32_t width, uint32_t height, char8_t const* window_title)
        :window(nullptr), surface(nullptr), graphics_instance(nullptr), graphics_window(nullptr),
         default_key_handler_guard(installDefaultKeyHandler()),
         default_resize_handler_guard(installDefaultResizeHandler())
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
        graphics_window = &ngraphics_window;
        VkInstance const vk_instance = instance.getVulkanInstance().getVkInstance();
        VkResult const res = glfwCreateWindowSurface(vk_instance, window, nullptr, &surface);
        if (res != VK_SUCCESS) {
            GHULBUS_THROW(Exceptions::GLFWError{}, "Unable to initialize Vulkan context.");
        }

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, GLFW_Pimpl::static_keyCallback);
        glfwSetCharCallback(window, GLFW_Pimpl::static_textCallback);
        glfwSetCursorPosCallback(window, GLFW_Pimpl::static_mouseMoveCallback);
        glfwSetCursorEnterCallback(window, GLFW_Pimpl::static_mouseEnterCallback);
        glfwSetMouseButtonCallback(window, GLFW_Pimpl::static_mouseClickCallback);
        glfwSetScrollCallback(window, GLFW_Pimpl::static_mouseScrollCallback);
        glfwSetFramebufferSizeCallback(window, GLFW_Pimpl::static_viewportResizeCallback);
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

    static void static_textCallback(GLFWwindow* window, unsigned int codepoint)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->textCallback(codepoint);
    }

    static void static_mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->mouseMoveCallback(xpos, ypos);
    }

    static void static_mouseEnterCallback(GLFWwindow* window, int did_enter)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->mouseEnterCallback(did_enter);
    }

    static void static_mouseClickCallback(GLFWwindow* window, int button, int action, int mods)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->mouseClickCallback(button, action, mods);
    }

    static void static_mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->mouseScrollCallback(xoffset, yoffset);
    }

    static void static_viewportResizeCallback(GLFWwindow* window, int width, int height)
    {
        GLFW_Pimpl* thisptr = reinterpret_cast<GLFW_Pimpl*>(glfwGetWindowUserPointer(window));
        GHULBUS_ASSERT(thisptr->window == window);
        thisptr->viewportResizeCallback(width, height);
    }

    void keyCallback(int key, int scancode, int action, int mods)
    {
        Event::Key key_event;
        key_event.key = static_cast<Key>(key);
        key_event.action = static_cast<KeyAction>(action);
        key_event.modifiers = static_cast<ModifierFlag>(mods);
        GHULBUS_UNUSED_VARIABLE(scancode);
        event_reactor.onKey(key_event);
    }

    void textCallback(unsigned int codepoint)
    {
        Event::Text text_event;
        text_event.codepoint = codepoint;
        event_reactor.onText(text_event);
    }

    void mouseMoveCallback(double xpos, double ypos)
    {
        Event::MouseMove mouse_move_event;
        mouse_move_event.position = GhulbusMath::Vector2d(xpos, ypos);
        event_reactor.onMouseMove(mouse_move_event);
    }

    void mouseEnterCallback(int did_enter)
    {
        if(did_enter == GLFW_TRUE) {
            event_reactor.onMouseEnter({});
        } else {
            GHULBUS_ASSERT(did_enter == GLFW_FALSE);
            event_reactor.onMouseLeave({});
        }
    }

    void mouseClickCallback(int button, int action, int mods)
    {
        Event::MouseClick mouse_click_event;
        mouse_click_event.button = static_cast<MouseButton>(button);
        mouse_click_event.action = static_cast<MouseButtonAction>(action);
        mouse_click_event.modifiers = static_cast<ModifierFlag>(mods);
        event_reactor.onMouseClick(mouse_click_event);
    }

    void mouseScrollCallback(double xoffset, double yoffset)
    {
        Event::MouseScroll mouse_scroll_event;
        mouse_scroll_event.offset = GhulbusMath::Vector2d(xoffset, yoffset);
        event_reactor.onMouseScroll(mouse_scroll_event);
    }

    void viewportResizeCallback(int width, int height)
    {
        Event::ViewportResize resize_event;
        resize_event.new_width = static_cast<uint32_t>(width);
        resize_event.new_height = static_cast<uint32_t>(height);
        event_reactor.onViewportResize(resize_event);
    }

    WindowEventReactor::KeyHandlerGuard installDefaultKeyHandler()
    {
        return event_reactor.eventHandlers.keyEvent.addHandler(
            [this](Event::Key const& key_event) -> WindowEventReactor::Result
            {
                if (key_event.action == KeyAction::Press) {
                    if (key_event.key == Key::Escape) {
                        glfwSetWindowShouldClose(window, true);
                    } else if(key_event.key == Key::M) {
                        if(!graphics_window->isCursorDisabled()) {
                            graphics_window->disableCursor(true);
                            graphics_window->setMouseMotionRaw(true);
                        } else {
                            graphics_window->disableCursor(false);
                            graphics_window->setMouseMotionRaw(false);
                        }
                    }
                }
                return WindowEventReactor::Result::ContinueProcessing;
            });
    }

    WindowEventReactor::ViewportResizeGuard installDefaultResizeHandler()
    {
        return event_reactor.eventHandlers.viewportResizeEvent.addHandler(
            [this](Event::ViewportResize const& resize_event) -> WindowEventReactor::Result
            {
                resized_to = VkExtent2D{ resize_event.new_width, resize_event.new_height };
                graphics_window->onResize(resize_event.new_width, resize_event.new_height);
                return WindowEventReactor::Result::ContinueProcessing;
            }
        );
    }
};

Window::Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title)
    :m_width(width), m_height(height), m_glfw(std::make_unique<GLFW_Pimpl>(instance, *this, width, height, window_title)),
    m_swapchain(instance.getVulkanDevice().createSwapchain(m_glfw->surface, instance.getGraphicsQueueFamilyIndex())),
    m_presentCommandBuffers(m_glfw->graphics_instance->getCommandPoolRegistry().allocateCommandBuffersGraphics(m_swapchain.getNumberOfImages())),
    m_presentQueue(&m_glfw->graphics_instance->getGraphicsQueue()),
    m_presentFence(m_glfw->graphics_instance->getVulkanDevice().createFence())
{
    m_swapchain.setDebugName(std::format("gbGraphics.Window.('{}')", reinterpret_cast<char const*>(window_title)).c_str());
    m_presentFence.setDebugName(std::format("gbGraphics.Present.('{}')", reinterpret_cast<char const*>(window_title)).c_str());
    prepareBackbuffer();
}

Window::~Window()
{}

void Window::close()
{
    glfwSetWindowShouldClose(m_glfw->window, GLFW_TRUE);
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

WindowEventReactor& Window::getEventReactor()
{
    return m_glfw->event_reactor;
}

Window::PresentStatus Window::present(GhulbusVulkan::Semaphore& render_finished_semaphore)
{
    GHULBUS_PRECONDITION(m_backBuffer);

    m_presentQueue->stageSubmission(std::move(m_windowSubmits));
    m_windowSubmits = GhulbusVulkan::SubmitStaging{};
    m_presentFence.reset();
    m_presentQueue->submitAllStaged(m_presentFence);
    try {
        m_swapchain.present(m_presentQueue->getVkQueue(), render_finished_semaphore, std::move(m_backBuffer->image));
    } catch(GhulbusVulkan::Exceptions::VulkanError const& e) {
        VkResult const* const res = Ghulbus::getErrorInfo<GhulbusVulkan::Exception_Info::vulkan_error_code>(e);
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

void Window::disableCursor(bool do_disable)
{
    glfwSetInputMode(m_glfw->window, GLFW_CURSOR, (do_disable ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL));
}

bool Window::isCursorDisabled() const
{
    int const res = glfwGetInputMode(m_glfw->window, GLFW_CURSOR);
    return (res == GLFW_CURSOR_DISABLED);
}

void Window::setMouseMotionRaw(bool do_raw_input)
{
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_glfw->window, GLFW_RAW_MOUSE_MOTION, (do_raw_input ? GLFW_TRUE : GLFW_FALSE));
    }
}

GhulbusMath::Vector2d Window::getMousePosition()
{
    GhulbusMath::Vector2d ret;
    glfwGetCursorPos(m_glfw->window, &ret.x, &ret.y);
    return ret;
}

KeyState Window::getKeyState(Key k)
{
    return static_cast<KeyState>(glfwGetKey(m_glfw->window, static_cast<int>(k)));
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
    return m_backBuffer->semaphores[m_backBuffer->currentSemaphoreIndex];
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
    m_recreateCallbacks.emplace_back(std::move(cb));
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
    for(auto const& cb : m_recreateCallbacks) { cb(m_swapchain); }
}

void Window::prepareBackbuffer()
{
    if (!m_backBuffer) {
        std::vector<GhulbusVulkan::Semaphore> backbuffer_semaphores;
        uint32_t const n_swapchain_images = m_swapchain.getNumberOfImages();
        if (n_swapchain_images == 0) { return; }
        backbuffer_semaphores.reserve(n_swapchain_images);
        for (uint32_t i = 0; i < n_swapchain_images; ++i) {
            backbuffer_semaphores.emplace_back(m_glfw->graphics_instance->getVulkanDevice().createSemaphore());
            backbuffer_semaphores.back().setDebugName(std::format("gbGraphics.ImageAcquire#{}", i).c_str());
        }
        GhulbusVulkan::Swapchain::AcquiredImage image = m_swapchain.acquireNextImage(backbuffer_semaphores.front());
        m_backBuffer.emplace(Backbuffer{ std::move(image), std::move(backbuffer_semaphores), 0 });
    } else {
        try {
            m_backBuffer->currentSemaphoreIndex = (m_backBuffer->currentSemaphoreIndex + 1) % m_backBuffer->semaphores.size();
            auto& current_semaphore = m_backBuffer->semaphores[m_backBuffer->currentSemaphoreIndex];
            m_backBuffer->image = m_swapchain.acquireNextImage(current_semaphore);
        } catch(GhulbusVulkan::Exceptions::VulkanError const& e) {
            VkResult const* const res = Ghulbus::getErrorInfo<GhulbusVulkan::Exception_Info::vulkan_error_code>(e);
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

void Window::onResize(uint32_t new_width, uint32_t new_height)
{
    m_width = static_cast<int>(new_width);
    m_height = static_cast<int>(new_height);
}

GLFWwindow* Window::getGlfwWindow()
{
    return m_glfw->window;
}
}
