#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_RENDERER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_RENDERER_HPP

/** @file
*
* @brief Renderer.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/GenericImage.hpp>

#include <gbVk/ForwardDecl.hpp>
#include <gbVk/Framebuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/ImageView.hpp>
#include <gbVk/Pipeline.hpp>
#include <gbVk/RenderPass.hpp>

#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class Image2d;

class Renderer {
private:
    GraphicsInstance* m_instance;
    Image2d* m_target;
    GhulbusVulkan::CommandBuffers m_rendererCommands;
    uint32_t m_currentFrame;
    GenericImage m_depthBuffer;
    GhulbusVulkan::ImageView m_depthBufferImageView;
    GhulbusVulkan::RenderPass m_renderPass;
    std::vector<GhulbusVulkan::Framebuffer> m_framebuffers;
public:
    Renderer(GraphicsInstance& instance, GhulbusVulkan::Swapchain& swapchain);

    void beginRender(Image2d& target_image);
    void endRender();

    GhulbusVulkan::RenderPass& getRenderPass();
    GhulbusVulkan::Framebuffer& getFramebufferByIndex(uint32_t idx);

private:
    static GenericImage createDepthBuffer(GraphicsInstance& instance, uint32_t width, uint32_t height);
    static bool isStencilFormat(VkFormat format);
    static GhulbusVulkan::ImageView createDepthBufferImageView(GenericImage& depth_buffer);
    static GhulbusVulkan::RenderPass createRenderPass(GraphicsInstance& instance,
                                                      VkFormat target_format, VkFormat depth_buffer_format);
    static auto createFramebuffers(GraphicsInstance& instance, GhulbusVulkan::Swapchain& swapchain,
                                   GhulbusVulkan::RenderPass& render_pass,
                                   GhulbusVulkan::ImageView& depth_image_view)
        -> std::vector<GhulbusVulkan::Framebuffer>;
};
}
#endif
