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
#include <gbVk/PipelineBuilder.hpp>
#include <gbVk/PipelineLayout.hpp>
#include <gbVk/RenderPass.hpp>

#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class Image2d;
class Program;

class Renderer {
private:
    struct PipelineBuildingBlocks {
        GhulbusVulkan::PipelineBuilder builder;
        GhulbusVulkan::PipelineLayout layout;

        PipelineBuildingBlocks(GhulbusVulkan::PipelineBuilder&& b, GhulbusVulkan::PipelineLayout&& l)
            :builder(std::move(b)), layout(std::move(l))
        {}
    };
private:
    GraphicsInstance* m_instance;
    Program* m_program;
    GhulbusVulkan::Swapchain* m_swapchain;
    Image2d* m_target;
    GhulbusVulkan::CommandBuffers m_rendererCommands;
    uint32_t m_currentFrame;
    GenericImage m_depthBuffer;
    GhulbusVulkan::ImageView m_depthBufferImageView;
    GhulbusVulkan::RenderPass m_renderPass;
    std::vector<GhulbusVulkan::Framebuffer> m_framebuffers;
    std::vector<PipelineBuildingBlocks> m_pipelineBuilders;
    std::vector<GhulbusVulkan::Pipeline> m_pipelines;
public:
    Renderer(GraphicsInstance& instance, Program& program, GhulbusVulkan::Swapchain& swapchain);

    void beginRender(Image2d& target_image);
    void endRender();

    uint32_t addPipelineBuilder(GhulbusVulkan::PipelineLayout&& layout);
    GhulbusVulkan::PipelineBuilder& getPipelineBuilder(uint32_t index);
    GhulbusVulkan::PipelineLayout& getPipelineLayout(uint32_t index);
    void recreateAllPipelines();
    GhulbusVulkan::Pipeline& getPipeline(uint32_t index);

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
