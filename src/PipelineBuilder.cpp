#include <gbVk/PipelineBuilder.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

PipelineBuilder::Stages::Viewport::Viewport(Viewport const& rhs)
    :viewports(rhs.viewports), scissors(rhs.scissors), ci(rhs.ci)
{
    refreshReferences();
}

PipelineBuilder::Stages::Viewport& PipelineBuilder::Stages::Viewport::operator=(Viewport const& rhs)
{
    if (&rhs != this) {
        viewports = rhs.viewports;
        scissors = rhs.scissors;
        ci = rhs.ci;
        refreshReferences();
    }
    return *this;
}

PipelineBuilder::Stages::Viewport::Viewport(Viewport&& rhs)
    :viewports(std::move(rhs.viewports)), scissors(std::move(rhs.scissors)), ci(std::move(rhs.ci))
{
    refreshReferences();
}

PipelineBuilder::Stages::Viewport& PipelineBuilder::Stages::Viewport::operator==(Viewport&& rhs)
{
    if (&rhs != this) {
        viewports = std::move(rhs.viewports);
        scissors = std::move(rhs.scissors);
        ci = std::move(rhs.ci);
        refreshReferences();
    }
    return *this;
}

void PipelineBuilder::Stages::Viewport::refreshReferences()
{
    ci.viewportCount = static_cast<std::uint32_t>(viewports.size());
    ci.pViewports = viewports.data();
    ci.scissorCount = static_cast<std::uint32_t>(scissors.size());
    ci.pScissors = scissors.data();
}

PipelineBuilder::Stages::ColorBlend::ColorBlend(ColorBlend const& rhs)
    :attachments(rhs.attachments), ci(rhs.ci)
{
    refreshReferences();
}

PipelineBuilder::Stages::ColorBlend& PipelineBuilder::Stages::ColorBlend::operator=(ColorBlend const& rhs)
{
    if (&rhs != this) {
        attachments = rhs.attachments;
        ci = rhs.ci;
        refreshReferences();
    }
    return *this;
}

PipelineBuilder::Stages::ColorBlend::ColorBlend(ColorBlend&& rhs)
    :attachments(std::move(rhs.attachments)), ci(std::move(rhs.ci))
{
    refreshReferences();
}

PipelineBuilder::Stages::ColorBlend& PipelineBuilder::Stages::ColorBlend::operator=(ColorBlend&& rhs)
{
    if (&rhs != this) {
        attachments = std::move(rhs.attachments);
        ci = std::move(rhs.ci);
        refreshReferences();
    }
    return *this;
}

void PipelineBuilder::Stages::ColorBlend::refreshReferences()
{
    ci.attachmentCount = static_cast<uint32_t>(attachments.size());
    ci.pAttachments = attachments.data();
}

PipelineBuilder PipelineBuilder::graphicsPipeline(int viewport_width, int viewport_height)
{
    PipelineBuilder ret;
    ret.stage.vertex_input = VkPipelineVertexInputStateCreateInfo{};
    ret.stage.vertex_input->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    ret.stage.vertex_input->pNext = nullptr;
    ret.stage.vertex_input->flags = 0;
    ret.stage.vertex_input->vertexBindingDescriptionCount = 0;
    ret.stage.vertex_input->pVertexBindingDescriptions = nullptr;
    ret.stage.vertex_input->vertexAttributeDescriptionCount = 0;
    ret.stage.vertex_input->pVertexAttributeDescriptions = nullptr;

    ret.stage.input_assembly = VkPipelineInputAssemblyStateCreateInfo{};
    ret.stage.input_assembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ret.stage.input_assembly->pNext = nullptr;
    ret.stage.input_assembly->flags = 0;
    ret.stage.input_assembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    ret.stage.input_assembly->primitiveRestartEnable = VK_FALSE;

    ret.stage.viewport = PipelineBuilder::Stages::Viewport{};
    ret.stage.viewport->viewports.push_back(VkViewport{});
    ret.stage.viewport->viewports.back().x = 0;
    ret.stage.viewport->viewports.back().y = 0;
    ret.stage.viewport->viewports.back().width = static_cast<float>(viewport_width);
    ret.stage.viewport->viewports.back().height = static_cast<float>(viewport_height);
    ret.stage.viewport->viewports.back().minDepth = 0.f;
    ret.stage.viewport->viewports.back().maxDepth = 1.f;
    ret.stage.viewport->scissors.push_back(VkRect2D{});
    ret.stage.viewport->scissors.back().offset.x = 0;
    ret.stage.viewport->scissors.back().offset.y = 0;
    ret.stage.viewport->scissors.back().extent.width = viewport_width;
    ret.stage.viewport->scissors.back().extent.height = viewport_height;

    ret.stage.viewport->ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ret.stage.viewport->ci.pNext = nullptr;
    ret.stage.viewport->ci.flags = 0;
    ret.stage.viewport->ci.viewportCount = static_cast<std::uint32_t>(ret.stage.viewport->viewports.size());
    ret.stage.viewport->ci.pViewports = ret.stage.viewport->viewports.data();
    ret.stage.viewport->ci.scissorCount = static_cast<std::uint32_t>(ret.stage.viewport->scissors.size());
    ret.stage.viewport->ci.pScissors = ret.stage.viewport->scissors.data();

    ret.stage.rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    ret.stage.rasterization.pNext = nullptr;
    ret.stage.rasterization.flags = 0;
    ret.stage.rasterization.depthClampEnable = VK_FALSE;
    ret.stage.rasterization.rasterizerDiscardEnable = VK_FALSE;
    ret.stage.rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    ret.stage.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
    ret.stage.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    ret.stage.rasterization.depthBiasEnable = VK_FALSE;
    ret.stage.rasterization.depthBiasConstantFactor = 0.f;
    ret.stage.rasterization.depthBiasClamp = 0.f;
    ret.stage.rasterization.depthBiasSlopeFactor = 0.f;
    ret.stage.rasterization.lineWidth = 1.f;

    ret.stage.multisample = VkPipelineMultisampleStateCreateInfo{};
    ret.stage.multisample->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ret.stage.multisample->pNext = nullptr;
    ret.stage.multisample->flags = 0;
    ret.stage.multisample->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ret.stage.multisample->sampleShadingEnable = VK_FALSE;
    ret.stage.multisample->minSampleShading = 1.f;
    ret.stage.multisample->pSampleMask = nullptr;
    ret.stage.multisample->alphaToCoverageEnable = VK_FALSE;
    ret.stage.multisample->alphaToOneEnable = VK_FALSE;

    ret.stage.depth_stencil = std::nullopt;

    ret.stage.color_blend = PipelineBuilder::Stages::ColorBlend{};
    ret.stage.color_blend->attachments.push_back(VkPipelineColorBlendAttachmentState{});
    ret.stage.color_blend->attachments.back().blendEnable = VK_FALSE;
    ret.stage.color_blend->attachments.back().srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    ret.stage.color_blend->attachments.back().dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    ret.stage.color_blend->attachments.back().colorBlendOp = VK_BLEND_OP_ADD;
    ret.stage.color_blend->attachments.back().srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ret.stage.color_blend->attachments.back().dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ret.stage.color_blend->attachments.back().alphaBlendOp = VK_BLEND_OP_ADD;
    ret.stage.color_blend->attachments.back().colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                               VK_COLOR_COMPONENT_G_BIT |
                                                               VK_COLOR_COMPONENT_B_BIT |
                                                               VK_COLOR_COMPONENT_A_BIT;
    ret.stage.color_blend->ci = VkPipelineColorBlendStateCreateInfo{};
    ret.stage.color_blend->ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ret.stage.color_blend->ci.pNext = nullptr;
    ret.stage.color_blend->ci.flags = 0;
    ret.stage.color_blend->ci.logicOpEnable = VK_FALSE;
    ret.stage.color_blend->ci.logicOp = VK_LOGIC_OP_COPY;
    ret.stage.color_blend->ci.attachmentCount = static_cast<uint32_t>(ret.stage.color_blend->attachments.size());
    ret.stage.color_blend->ci.pAttachments = ret.stage.color_blend->attachments.data();
    ret.stage.color_blend->ci.blendConstants[0] = 0.f;
    ret.stage.color_blend->ci.blendConstants[1] = 0.f;
    ret.stage.color_blend->ci.blendConstants[2] = 0.f;
    ret.stage.color_blend->ci.blendConstants[3] = 0.f;

    return ret;
}

}


