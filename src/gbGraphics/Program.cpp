#include <gbGraphics/Program.hpp>

#include <gbGraphics/GraphicsInstance.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/SpirvCode.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Program::Program(GraphicsInstance& instance, GhulbusVulkan::SpirvCode const& vertex_shader,
                 GhulbusVulkan::SpirvCode const& fragment_shader)
    :m_instance(&instance), m_vertexShader(instance.getVulkanDevice().createShaderModule(vertex_shader)),
    m_fragmentShader(instance.getVulkanDevice().createShaderModule(fragment_shader))
{
    m_shaderStageCreateInfos = createShaderStageCreateInfos();
}

VkPipelineShaderStageCreateInfo const* Program::getShaderStageCreateInfos() const
{
    return m_shaderStageCreateInfos.data();
}

uint32_t Program::getNumberOfShaderStages() const
{
    return static_cast<uint32_t>(m_shaderStageCreateInfos.size());
}

std::vector<VkPipelineShaderStageCreateInfo> Program::createShaderStageCreateInfos()
{
    std::vector<VkPipelineShaderStageCreateInfo> ret;
    ret.resize(2);

    VkPipelineShaderStageCreateInfo& vert_shader_stage_ci = ret[0];
    vert_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_ci.pNext = nullptr;
    vert_shader_stage_ci.flags = 0;
    vert_shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_ci.module = m_vertexShader.getVkShaderModule();
    vert_shader_stage_ci.pName = "main";
    vert_shader_stage_ci.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo& frag_shader_stage_ci = ret[1];
    frag_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_ci.pNext = nullptr;
    frag_shader_stage_ci.flags = 0;
    frag_shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_ci.module = m_fragmentShader.getVkShaderModule();
    frag_shader_stage_ci.pName = "main";
    frag_shader_stage_ci.pSpecializationInfo = nullptr;

    return ret;
}

}
