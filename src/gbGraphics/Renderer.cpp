#include <gbGraphics/Renderer.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/Image2d.hpp>
#include <gbGraphics/Program.hpp>

#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/RenderPassBuilder.hpp>
#include <gbVk/Swapchain.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Renderer::Renderer(GraphicsInstance& instance, Program& program, GhulbusVulkan::Swapchain& swapchain)
    :m_instance(&instance), m_program(&program), m_swapchain(&swapchain), m_target(nullptr),
    m_rendererCommands(instance.getCommandPoolRegistry().allocateCommandBuffersGraphics(swapchain.getNumberOfImages())),
    m_currentFrame(0), m_depthBuffer(createDepthBuffer(instance, swapchain.getWidth(), swapchain.getHeight())),
    m_depthBufferImageView(createDepthBufferImageView(m_depthBuffer)),
    m_renderPass(createRenderPass(instance, swapchain.getFormat(), m_depthBuffer.getFormat())),
    m_framebuffers(createFramebuffers(instance, swapchain, m_renderPass, m_depthBufferImageView))
{}

void Renderer::beginRender(Image2d& target_image)
{
    // GHULBUS_PRECONDITION(target_image.getWidth() == m_depthBuffer->getExtent().width);
    // GHULBUS_PRECONDITION(target_image.getHeight() == m_depthBuffer->getExtent().height);
}

void Renderer::endRender()
{
    ++m_currentFrame;
}

uint32_t Renderer::addPipelineBuilder(GhulbusVulkan::PipelineLayout&& layout)
{
    m_pipelineBuilders.emplace_back(
        m_instance->getVulkanDevice().createGraphicsPipelineBuilder(m_swapchain->getWidth(), m_swapchain->getHeight()),
        std::move(layout));
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
    return m_pipelineBuilders[index].layout;
}

void Renderer::recreateAllPipelines()
{
    m_pipelines.clear();
    for (auto& [builder, layout] : m_pipelineBuilders) {
        m_pipelines.emplace_back(
            builder.create(layout, m_program->getShaderStageCreateInfos(), m_program->getNumberOfShaderStages(),
                           m_renderPass.getVkRenderPass()));
    }
}

GhulbusVulkan::Pipeline& Renderer::getPipeline(uint32_t index)
{
    GHULBUS_PRECONDITION((index >= 0) && (index < m_pipelines.size()));
    return m_pipelines[index];
}

GhulbusVulkan::RenderPass& Renderer::getRenderPass()
{
    return m_renderPass;
}

GhulbusVulkan::Framebuffer& Renderer::getFramebufferByIndex(uint32_t idx)
{
    GHULBUS_PRECONDITION((idx >= 0) && (idx < m_framebuffers.size()));
    return m_framebuffers[idx];
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
}
