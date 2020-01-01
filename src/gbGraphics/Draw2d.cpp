#include <gbGraphics/Draw2d.hpp>

#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/Window.hpp>

#include <gbGraphics/detail/CompiledShaders.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/PipelineLayout.hpp>
#include <gbVk/PipelineLayoutBuilder.hpp>
#include <gbVk/SpirvCode.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
namespace
{
GhulbusVulkan::SpirvCode loadShader(detail::shader::ShaderData (*shader_data_func)())
{
    detail::shader::ShaderData const shader_data = shader_data_func();
    return GhulbusVulkan::SpirvCode::load(shader_data.data, shader_data.size);
}

GhulbusVulkan::PipelineLayout createPipelineLayout(GraphicsInstance& instance)
{
    auto layout_builder = instance.getVulkanDevice().createPipelineLayoutBuilder();
    return layout_builder.create();
}
}

Draw2d::Draw2d(GraphicsInstance& instance, Window& target_window)
    :m_program(instance, loadShader(detail::shader::draw2dVertex), loadShader(detail::shader::draw2dFragment)),
     m_renderer(instance, m_program, target_window.getSwapchain()), m_window(&target_window)
{
    m_renderer.addPipelineBuilder(createPipelineLayout(instance));
    m_renderer.setClearColor(GhulbusMath::Color4f(0.f, 0.f, 0.f, 1.f));
    m_renderer.recreateAllPipelines();
}

void Draw2d::draw()
{
    m_renderer.render(0, *m_window);
}

Renderer& Draw2d::getRenderer()
{
    return m_renderer;
}
}
