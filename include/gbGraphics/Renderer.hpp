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
#include <gbVk/Semaphore.hpp>

#include <gbMath/Color4.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class Image2d;
class Program;
class Window;

class Renderer {
public:
    using DrawRecordingCallback = std::function<void(GhulbusVulkan::CommandBuffer&, uint32_t)>;
private:
    struct PipelineBuildingBlocks {
        GhulbusVulkan::PipelineBuilder builder;
        std::shared_ptr<GhulbusVulkan::PipelineLayout> layout;

        PipelineBuildingBlocks(GhulbusVulkan::PipelineBuilder&& b, GhulbusVulkan::PipelineLayout&& l)
            :builder(std::move(b)), layout(std::make_shared<GhulbusVulkan::PipelineLayout>(std::move(l)))
        {}
    };
    struct RendererState {
        GenericImage depthBuffer;
        GhulbusVulkan::ImageView depthBufferImageView;
        GhulbusVulkan::RenderPass renderPass;
        std::vector<GhulbusVulkan::Framebuffer> framebuffers;

        RendererState(GenericImage&& depth_buffer, GhulbusVulkan::ImageView&& depth_buffer_image_view,
                      GhulbusVulkan::RenderPass&& render_pass, std::vector<GhulbusVulkan::Framebuffer> n_framebuffers);
        RendererState(RendererState&&) = default;
    };
private:
    GraphicsInstance* m_instance;
    Program* m_program;
    GhulbusVulkan::Swapchain* m_swapchain;
    GhulbusVulkan::CommandBuffers m_commandBuffers;
    std::optional<RendererState> m_state;
    std::vector<PipelineBuildingBlocks> m_pipelineBuilders;
    std::vector<GhulbusVulkan::Pipeline> m_pipelines;
    GhulbusVulkan::Semaphore m_renderFinishedSemaphore;

    std::vector<std::vector<DrawRecordingCallback>> m_drawRecordings;   ///< one vector per pipeline

    GhulbusMath::Color4f m_clearColor;
public:
    Renderer(GraphicsInstance& instance, Program& program, GhulbusVulkan::Swapchain& swapchain);

    uint32_t addPipelineBuilder(GhulbusVulkan::PipelineLayout&& layout);
    uint32_t clonePipelineBuilder(uint32_t source_index);
    GhulbusVulkan::PipelineBuilder& getPipelineBuilder(uint32_t index);
    GhulbusVulkan::PipelineLayout& getPipelineLayout(uint32_t index);
    void recreateAllPipelines();
    GhulbusVulkan::Pipeline& getPipeline(uint32_t index);

    uint32_t recordDrawCommands(uint32_t pipeline_index, DrawRecordingCallback const& recording_cb);
    void forceInvokeDrawCallback(uint32_t pipeline_index, uint32_t target_index);       /// @todo this is a hack for imgui
    uint32_t copyDrawCommands(uint32_t source_pipeline_index, uint32_t source_draw_command_index,
                              uint32_t destination_pipeline_index);

    void render(uint32_t pipeline_index, Window& target_window);

    GhulbusVulkan::RenderPass& getRenderPass();
    GhulbusVulkan::Framebuffer& getFramebufferByIndex(uint32_t idx);

    void setClearColor(GhulbusMath::Color4f const& clear_color);

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
    static RendererState createRendererState(GraphicsInstance& instance, GhulbusVulkan::Swapchain& swapchain);
    uint32_t getCommandBufferIndex(uint32_t pipeline_index, uint32_t target_index) const;
};
}
#endif
