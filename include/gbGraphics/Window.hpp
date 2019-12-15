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
#include <gbVk/SubmitStaging.hpp>
#include <gbVk/Swapchain.hpp>

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

    struct DoNotWait_T {};
private:
    int m_width;
    int m_height;
    struct GLFW_Pimpl;
    std::unique_ptr<GLFW_Pimpl> m_glfw;
    std::optional<Backbuffer> m_backBuffer;         // this must be destroyed after m_swapchain to avoid races on the semaphore in Backbuffer
    GhulbusVulkan::Swapchain m_swapchain;
    GhulbusVulkan::CommandBuffers m_presentCommandBuffers;
    GhulbusVulkan::SubmitStaging m_windowSubmits;
    GhulbusVulkan::Queue* m_presentQueue;
    GhulbusVulkan::Fence m_presentFence;
public:
    Window(GraphicsInstance& instance, int width, int height, char8_t const* window_title);
    ~Window();

    bool isDone();

    int getWidth() const;
    int getHeight() const;

    void present();

    void present(DoNotWait_T);

    void present(GhulbusVulkan::Semaphore& semaphore);

    void present(GhulbusVulkan::Semaphore& semaphore, DoNotWait_T);

    uint32_t getNumberOfImagesInSwapchain() const;

    uint32_t getCurrentImageSwapchainIndex() const;

    GhulbusVulkan::Semaphore& getCurrentImageAcquireSemaphore();

    GhulbusVulkan::Swapchain& getSwapchain();

    ///@todo remove?
    GhulbusVulkan::Swapchain::AcquiredImage& getAcquiredImage();

private:
    void prepareBackbuffer();
};
}
#endif
