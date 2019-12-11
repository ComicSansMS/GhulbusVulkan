#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_WINDOW_HPP

/** @file
*
* @brief Graphics Window.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/Semaphore.hpp>
#include <gbVk/Swapchain.hpp>
#include <gbVk/Queue.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;

class Window {
private:
    struct Backbuffer {
        GhulbusVulkan::Swapchain::AcquiredImage image;
        GhulbusVulkan::Fence fence;
        GhulbusVulkan::Semaphore semaphore;
    };
private:
    uint32_t m_width;
    uint32_t m_height;
    struct GLFW_Pimpl;
    std::unique_ptr<GLFW_Pimpl> m_glfw;
    std::optional<Backbuffer> m_backBuffer;         // this must be destroyed after m_swapchain to avoid races on the semaphore in Backbuffer
    GhulbusVulkan::Swapchain m_swapchain;
    GhulbusVulkan::CommandBuffers m_presentCommandBuffers;
    GhulbusVulkan::Queue m_presentQueue;
public:
    Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title);
    ~Window();

    bool isDone();

    uint32_t getWidth() const;
    uint32_t getHeight() const;

    void present();

    GhulbusVulkan::Swapchain& getSwapchain();

private:
    void prepareBackbuffer();
};
}
#endif
