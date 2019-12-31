#include <gbGraphics/Renderer.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/Image2d.hpp>
#include <gbGraphics/Program.hpp>
#include <gbGraphics/Window.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/Queue.hpp>
#include <gbVk/RenderPassBuilder.hpp>
#include <gbVk/Swapchain.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Renderer::Renderer(GraphicsInstance& instance, Program& program, GhulbusVulkan::Swapchain& swapchain)
    :m_instance(&instance), m_program(&program), m_swapchain(&swapchain), m_target(nullptr),
     m_state(createRendererState(instance, swapchain)),
     m_renderFinishedSemaphore(instance.getVulkanDevice().createSemaphore())
{
}

uint32_t Renderer::addPipelineBuilder(GhulbusVulkan::PipelineLayout&& layout)
{
    m_pipelineBuilders.emplace_back(
        m_instance->getVulkanDevice().createGraphicsPipelineBuilder(m_swapchain->getWidth(), m_swapchain->getHeight()),
        std::move(layout));
    m_drawRecordings.emplace_back();
    GHULBUS_ASSERT(m_pipelineBuilders.size() == m_drawRecordings.size());
    return static_cast<uint32_t>(m_pipelineBuilders.size() - 1);
}

uint32_t Renderer::clonePipelineBuilder(uint32_t source_index)
{
    GHULBUS_PRECONDITION((source_index >= 0) && (source_index < m_pipelineBuilders.size()));
    m_pipelineBuilders.emplace_back(m_pipelineBuilders[source_index]);
    m_drawRecordings.emplace_back();
    GHULBUS_ASSERT(m_pipelineBuilders.size() == m_drawRecordings.size());
    return static_cast<uint32_t>(m_pipelineBuilders.size() - 1);
}

GhulbusVulkan::PipelineBuilder& Renderer::getPipelineBuilder(uint32_t index)
{
    GHULBUS_PRECONDITION((index >= 0) && (index < m_pipelineBuilders.size()));
    return m_pipelineBuilders[index].builder;
}

GhulbusVulkan::PipelineLayout& Renderer::getPipelineLayout(uint32_t index)
{
    GHULBUS_PRECONDITION((index >= 0) && (index < m_pipelineBuilders.size()));
    return *m_pipelineBuilders[index].layout;
}

void Renderer::recreateAllPipelines()
{
    GHULBUS_PRECONDITION(m_state);
    GHULBUS_PRECONDITION(!m_pipelineBuilders.empty());
    m_pipelines.clear();
    for (auto& [builder, layout] : m_pipelineBuilders) {
        builder.clearVertexBindings();
        auto const& program_bindings = m_program->getVertexInputBindings();
        auto const& program_attributes = m_program->getVertexInputAttributeDescriptions();
        builder.addVertexBindings(program_bindings.data(), static_cast<uint32_t>(program_bindings.size()),
                                  program_attributes.data(), static_cast<uint32_t>(program_attributes.size()));
        builder.adjustViewportDimensions(m_swapchain->getWidth(), m_swapchain->getHeight());
        m_pipelines.emplace_back(
            builder.create(*layout, m_program->getShaderStageCreateInfos(), m_program->getNumberOfShaderStages(),
                           m_state->renderPass.getVkRenderPass()));
    }
    uint32_t const n_pipelines = static_cast<uint32_t>(m_pipelineBuilders.size());
    uint32_t const n_targets = m_swapchain->getNumberOfImages();
    m_commandBuffers = m_instance->getCommandPoolRegistry().allocateCommandBuffersGraphics(n_targets * n_pipelines);
    GHULBUS_ASSERT(m_pipelineBuilders.size() == m_drawRecordings.size());
    GHULBUS_ASSERT(m_state->framebuffers.size() == n_targets);
    for (uint32_t pipeline_index = 0; pipeline_index < n_pipelines; ++pipeline_index) {
        for(uint32_t target_index = 0; target_index < n_targets; ++target_index) {
            GhulbusVulkan::CommandBuffer& command_buffer =
                m_commandBuffers.getCommandBuffer(getCommandBufferIndex(pipeline_index, target_index));

            command_buffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

            VkRenderPassBeginInfo render_pass_info;
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_info.pNext = nullptr;
            render_pass_info.renderPass = m_state->renderPass.getVkRenderPass();
            render_pass_info.framebuffer = m_state->framebuffers[target_index].getVkFramebuffer();
            render_pass_info.renderArea.offset.x = 0;
            render_pass_info.renderArea.offset.y = 0;
            render_pass_info.renderArea.extent.width = m_swapchain->getWidth();
            render_pass_info.renderArea.extent.height = m_swapchain->getHeight();
            std::array<VkClearValue, 2> clear_color;
            clear_color[0].color.float32[0] = 0.5f;
            clear_color[0].color.float32[1] = 0.f;
            clear_color[0].color.float32[2] = 0.5f;
            clear_color[0].color.float32[3] = 1.f;
            clear_color[1].depthStencil.depth = 1.0f;
            clear_color[1].depthStencil.stencil = 0;
            render_pass_info.clearValueCount = static_cast<uint32_t>(clear_color.size());
            render_pass_info.pClearValues = clear_color.data();

            vkCmdBeginRenderPass(command_buffer.getVkCommandBuffer(), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(command_buffer.getVkCommandBuffer(),
                              VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[pipeline_index].getVkPipeline());

            for(auto const& draw_cb : m_drawRecordings[pipeline_index]) {
                draw_cb(command_buffer, target_index);
            }

            vkCmdEndRenderPass(command_buffer.getVkCommandBuffer());
            command_buffer.end();
        }
    }
}

void Renderer::render(uint32_t pipeline_index, Window& target_window)
{
    GHULBUS_PRECONDITION((pipeline_index >= 0) && (pipeline_index < m_pipelines.size()));
    GHULBUS_ASSERT(m_pipelines.size() == m_drawRecordings.size());
    GhulbusVulkan::SubmitStaging loop_stage;
    loop_stage.addWaitingSemaphore(target_window.getCurrentImageAcquireSemaphore(),
                                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    uint32_t const frame_image_index = target_window.getCurrentImageSwapchainIndex();
    auto& command_buffer = m_commandBuffers.getCommandBuffer(getCommandBufferIndex(pipeline_index, frame_image_index));

    loop_stage.addCommandBuffer(command_buffer);
    loop_stage.addSignalingSemaphore(m_renderFinishedSemaphore);
    m_instance->getGraphicsQueue().stageSubmission(std::move(loop_stage));

    Window::PresentStatus const present_status = target_window.present(m_renderFinishedSemaphore);

    if(present_status != Window::PresentStatus::Ok) {
        target_window.recreateSwapchain();
        m_state.reset();
        m_state.emplace(createRendererState(*m_instance, *m_swapchain));
        recreateAllPipelines();
    }
    m_instance->getGraphicsQueue().clearAllStaged();
}

GhulbusVulkan::Pipeline& Renderer::getPipeline(uint32_t index)
{
    GHULBUS_PRECONDITION((index >= 0) && (index < m_pipelines.size()));
    return m_pipelines[index];
}

uint32_t Renderer::recordDrawCommands(uint32_t pipeline_index, DrawRecordingCallback const& recording_cb)
{
    GHULBUS_PRECONDITION((pipeline_index >= 0) && (pipeline_index < m_pipelineBuilders.size()));
    GHULBUS_ASSERT(m_drawRecordings.size() == m_pipelineBuilders.size());
    m_drawRecordings[pipeline_index].push_back(recording_cb);
    return static_cast<uint32_t>(m_drawRecordings[pipeline_index].size()) - 1;
}

uint32_t Renderer::copyDrawCommands(uint32_t source_pipeline_index, uint32_t source_draw_command_index,
                                    uint32_t destination_pipeline_index)
{
    GHULBUS_PRECONDITION((source_pipeline_index >= 0) && (source_pipeline_index < m_pipelineBuilders.size()));
    GHULBUS_PRECONDITION((destination_pipeline_index >= 0) &&
                         (destination_pipeline_index < m_pipelineBuilders.size()));
    GHULBUS_ASSERT(m_drawRecordings.size() == m_pipelineBuilders.size());
    auto const& src_commands = m_drawRecordings[source_pipeline_index];
    auto& dst_commands = m_drawRecordings[destination_pipeline_index];
    GHULBUS_PRECONDITION((source_draw_command_index >= 0) && (source_draw_command_index < src_commands.size()));
    dst_commands.push_back(src_commands[source_draw_command_index]);
    return static_cast<uint32_t>(dst_commands.size()) - 1;
}

GhulbusVulkan::RenderPass& Renderer::getRenderPass()
{
    GHULBUS_PRECONDITION(m_state);
    return m_state->renderPass;
}

GhulbusVulkan::Framebuffer& Renderer::getFramebufferByIndex(uint32_t idx)
{
    GHULBUS_PRECONDITION((idx >= 0) && (idx < m_state->framebuffers.size()));
    GHULBUS_PRECONDITION(m_state);
    return m_state->framebuffers[idx];
}

GenericImage Renderer::createDepthBuffer(GraphicsInstance& instance, uint32_t width, uint32_t height)
{
    GhulbusVulkan::PhysicalDevice physical_device = instance.getVulkanPhysicalDevice();
    auto const depth_buffer_opt_format = physical_device.findDepthBufferFormat();
    if (!depth_buffer_opt_format) {
        GHULBUS_THROW(GhulbusVulkan::Exceptions::VulkanError(),
                      "No supported depth buffer format found.");
    }
    VkFormat const depth_buffer_format = *depth_buffer_opt_format;
    return GenericImage(instance, VkExtent3D{ width, height, 1 }, depth_buffer_format, 1, 1, VK_SAMPLE_COUNT_1_BIT,
                        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, MemoryUsage::GpuOnly);
}

bool Renderer::isStencilFormat(VkFormat format)
{
    return (format == VK_FORMAT_D16_UNORM_S8_UINT) ||
        (format == VK_FORMAT_D24_UNORM_S8_UINT) ||
        (format == VK_FORMAT_D32_SFLOAT_S8_UINT);
}

GhulbusVulkan::ImageView Renderer::createDepthBufferImageView(GenericImage& depth_buffer)
{
    return depth_buffer.createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT |
        (isStencilFormat(depth_buffer.getFormat()) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0));
}

GhulbusVulkan::RenderPass Renderer::createRenderPass(GraphicsInstance& instance,
                                                     VkFormat target_format, VkFormat depth_buffer_format)
{
    GhulbusVulkan::RenderPassBuilder builder = instance.getVulkanDevice().createRenderPassBuilder();
    builder.addSubpassGraphics();
    builder.addColorAttachment(target_format);
    builder.addDepthStencilAttachment(depth_buffer_format);
    return builder.create();
}

std::vector<GhulbusVulkan::Framebuffer> Renderer::createFramebuffers(GraphicsInstance& instance,
                                                                     GhulbusVulkan::Swapchain& swapchain,
                                                                     GhulbusVulkan::RenderPass& render_pass,
                                                                     GhulbusVulkan::ImageView& depth_image_view)
{
    return instance.getVulkanDevice().createFramebuffers(swapchain, render_pass, depth_image_view);
}

uint32_t Renderer::getCommandBufferIndex(uint32_t pipeline_index, uint32_t target_index) const
{
    uint32_t const n_targets = m_swapchain->getNumberOfImages();
    return (pipeline_index * n_targets) + target_index;
}

Renderer::RendererState::RendererState(GenericImage&& depth_buffer, GhulbusVulkan::ImageView&& depth_buffer_image_view,
                                       GhulbusVulkan::RenderPass&& render_pass,
                                       std::vector<GhulbusVulkan::Framebuffer> n_framebuffers)
    :depthBuffer(std::move(depth_buffer)), depthBufferImageView(std::move(depth_buffer_image_view)),
     renderPass(std::move(render_pass)), framebuffers(std::move(n_framebuffers))
{}

Renderer::RendererState Renderer::createRendererState(GraphicsInstance& instance,
                                                      GhulbusVulkan::Swapchain& swapchain)
{
    GhulbusGraphics::GenericImage depth_buffer =
        createDepthBuffer(instance, swapchain.getWidth(), swapchain.getHeight());
    GhulbusVulkan::ImageView image_view = createDepthBufferImageView(depth_buffer);
    GhulbusVulkan::RenderPass render_pass =
        createRenderPass(instance, swapchain.getFormat(), depth_buffer.getFormat());
    std::vector<GhulbusVulkan::Framebuffer> framebuffers =
        createFramebuffers(instance, swapchain, render_pass, image_view);
    return RendererState(std::move(depth_buffer), std::move(image_view),
                         std::move(render_pass), std::move(framebuffers));
}
}
