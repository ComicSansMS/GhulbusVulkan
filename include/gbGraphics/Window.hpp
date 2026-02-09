#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP

/** @file
*
* @brief Graphics Window.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/WindowEvents.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/Semaphore.hpp>
#include <gbVk/SubmitStaging.hpp>
#include <gbVk/Swapchain.hpp>

#include <gbMath/Vector2.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct GLFWwindow;

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class WindowEventReactor;

class Window {
public:
    enum class [[nodiscard]] PresentStatus {
        Ok = 0,
        InvalidBackbuffer,              ///< the backbuffer needs to be recreated
        InvalidBackbufferLostFrame      ///< the backbuffer needs to be recreated *and* the last frame due for presentation was lost
    };
    using RecreateSwapchainCallback = std::function<void(GhulbusVulkan::Swapchain&)>;
private:
    struct Backbuffer {
        GhulbusVulkan::Swapchain::AcquiredImage image;
        std::vector<GhulbusVulkan::Semaphore> semaphores;
        size_t currentSemaphoreIndex;
        bool is_invalid;
    };
private:
    int m_width;
    int m_height;
    struct GLFW_Pimpl;
    std::unique_ptr<GLFW_Pimpl> m_glfw;
    std::optional<Backbuffer> m_backBuffer;         // this must be destroyed *after* m_swapchain to avoid races on the semaphore in Backbuffer
    GhulbusVulkan::Swapchain m_swapchain;
    GhulbusVulkan::CommandBuffers m_presentCommandBuffers;
    GhulbusVulkan::SubmitStaging m_windowSubmits;
    GhulbusVulkan::Queue* m_presentQueue;
    GhulbusVulkan::Fence m_presentFence;
    std::vector<RecreateSwapchainCallback> m_recreateCallbacks;
public:
    Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title);
    ~Window();

    void close();
    bool isDone();

    uint32_t getWidth() const;
    uint32_t getHeight() const;

    WindowEventReactor& getEventReactor();

    PresentStatus present(GhulbusVulkan::Semaphore& render_finished_semaphore);

    void disableCursor(bool do_disable);
    bool isCursorDisabled() const;
    void setMouseMotionRaw(bool do_raw_input);
    GhulbusMath::Vector2d getMousePosition();
    KeyState getKeyState(Key k);

    uint32_t getNumberOfImagesInSwapchain() const;

    uint32_t getCurrentImageSwapchainIndex() const;

    GhulbusVulkan::Semaphore& getCurrentImageAcquireSemaphore();

    GhulbusVulkan::Swapchain& getSwapchain();

    ///@todo remove?
    GhulbusVulkan::Swapchain::AcquiredImage& getAcquiredImage();

    void addRecreateSwapchainCallback(RecreateSwapchainCallback cb);
    void recreateSwapchain();

    GLFWwindow* getGlfwWindow();

private:
    void prepareBackbuffer();
    void onResize(uint32_t new_width, uint32_t new_height);
};
}
#endif
