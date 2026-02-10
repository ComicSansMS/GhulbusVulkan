#include <gbVk/PipelineBuilder.hpp>

#include <gbVk/Pipeline.hpp>
#include <gbVk/PipelineLayout.hpp>

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
    ci.pViewports = (!viewports.empty()) ? viewports.data() : nullptr;
    ci.scissorCount = static_cast<std::uint32_t>(scissors.size());
    ci.pScissors = (!scissors.empty()) ? scissors.data() : nullptr;
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
    ci.pAttachments = (!attachments.empty()) ? attachments.data() : nullptr;
}

void PipelineBuilder::Stages::refreshReferences()
{
    if (viewport) { viewport->refreshReferences(); }
    if (color_blend) { color_blend->refreshReferences(); }
}

PipelineBuilder::PipelineBuilder(VkDevice logical_device, uint32_t viewport_width, uint32_t viewport_height)
    :m_device(logical_device)
{
    stage.vertex_input = VkPipelineVertexInputStateCreateInfo{};
    stage.vertex_input->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    stage.vertex_input->pNext = nullptr;
    stage.vertex_input->flags = 0;
    stage.vertex_input->vertexBindingDescriptionCount = 0;
    stage.vertex_input->pVertexBindingDescriptions = nullptr;
    stage.vertex_input->vertexAttributeDescriptionCount = 0;
    stage.vertex_input->pVertexAttributeDescriptions = nullptr;

    stage.input_assembly = VkPipelineInputAssemblyStateCreateInfo{};
    stage.input_assembly->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    stage.input_assembly->pNext = nullptr;
    stage.input_assembly->flags = 0;
    stage.input_assembly->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    stage.input_assembly->primitiveRestartEnable = VK_FALSE;

    stage.viewport = PipelineBuilder::Stages::Viewport{};
    stage.viewport->viewports.push_back(VkViewport{});
    stage.viewport->viewports.back().x = 0;
    stage.viewport->viewports.back().y = 0;
    stage.viewport->viewports.back().width = static_cast<float>(viewport_width);
    stage.viewport->viewports.back().height = static_cast<float>(viewport_height);
    stage.viewport->viewports.back().minDepth = 0.f;
    stage.viewport->viewports.back().maxDepth = 1.f;
    stage.viewport->scissors.push_back(VkRect2D{});
    stage.viewport->scissors.back().offset.x = 0;
    stage.viewport->scissors.back().offset.y = 0;
    stage.viewport->scissors.back().extent.width = viewport_width;
    stage.viewport->scissors.back().extent.height = viewport_height;

    stage.viewport->ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    stage.viewport->ci.pNext = nullptr;
    stage.viewport->ci.flags = 0;
    stage.viewport->ci.viewportCount = static_cast<std::uint32_t>(stage.viewport->viewports.size());
    stage.viewport->ci.pViewports = (!stage.viewport->viewports.empty()) ? stage.viewport->viewports.data() : nullptr;
    stage.viewport->ci.scissorCount = static_cast<std::uint32_t>(stage.viewport->scissors.size());
    stage.viewport->ci.pScissors = (!stage.viewport->scissors.empty()) ? stage.viewport->scissors.data() : nullptr;

    stage.rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    stage.rasterization.pNext = nullptr;
    stage.rasterization.flags = 0;
    stage.rasterization.depthClampEnable = VK_FALSE;
    stage.rasterization.rasterizerDiscardEnable = VK_FALSE;
    stage.rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    stage.rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
    stage.rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;
    stage.rasterization.depthBiasEnable = VK_FALSE;
    stage.rasterization.depthBiasConstantFactor = 0.f;
    stage.rasterization.depthBiasClamp = 0.f;
    stage.rasterization.depthBiasSlopeFactor = 0.f;
    stage.rasterization.lineWidth = 1.f;

    stage.multisample = VkPipelineMultisampleStateCreateInfo{};
    stage.multisample->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    stage.multisample->pNext = nullptr;
    stage.multisample->flags = 0;
    stage.multisample->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    stage.multisample->sampleShadingEnable = VK_FALSE;
    stage.multisample->minSampleShading = 1.f;
    stage.multisample->pSampleMask = nullptr;
    stage.multisample->alphaToCoverageEnable = VK_FALSE;
    stage.multisample->alphaToOneEnable = VK_FALSE;

    stage.depth_stencil = VkPipelineDepthStencilStateCreateInfo{};
    stage.depth_stencil->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stage.depth_stencil->pNext = nullptr;
    stage.depth_stencil->flags = 0;
    stage.depth_stencil->depthTestEnable = VK_TRUE;
    stage.depth_stencil->depthWriteEnable = VK_TRUE;
    stage.depth_stencil->depthCompareOp = VK_COMPARE_OP_LESS;
    stage.depth_stencil->depthBoundsTestEnable = VK_FALSE;
    stage.depth_stencil->stencilTestEnable = VK_FALSE;
    stage.depth_stencil->front = {};
    stage.depth_stencil->back = {};
    stage.depth_stencil->minDepthBounds = 0.0f;
    stage.depth_stencil->maxDepthBounds = 1.0f;

    stage.color_blend = PipelineBuilder::Stages::ColorBlend{};
    stage.color_blend->attachments.push_back(VkPipelineColorBlendAttachmentState{});
    stage.color_blend->attachments.back().blendEnable = VK_FALSE;
    stage.color_blend->attachments.back().srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    stage.color_blend->attachments.back().dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    stage.color_blend->attachments.back().colorBlendOp = VK_BLEND_OP_ADD;
    stage.color_blend->attachments.back().srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    stage.color_blend->attachments.back().dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    stage.color_blend->attachments.back().alphaBlendOp = VK_BLEND_OP_ADD;
    stage.color_blend->attachments.back().colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                           VK_COLOR_COMPONENT_G_BIT |
                                                           VK_COLOR_COMPONENT_B_BIT |
                                                           VK_COLOR_COMPONENT_A_BIT;
    stage.color_blend->ci = VkPipelineColorBlendStateCreateInfo{};
    stage.color_blend->ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    stage.color_blend->ci.pNext = nullptr;
    stage.color_blend->ci.flags = 0;
    stage.color_blend->ci.logicOpEnable = VK_FALSE;
    stage.color_blend->ci.logicOp = VK_LOGIC_OP_COPY;
    stage.color_blend->ci.attachmentCount = static_cast<uint32_t>(stage.color_blend->attachments.size());
    stage.color_blend->ci.pAttachments = (!stage.color_blend->attachments.empty()) ?
                                         stage.color_blend->attachments.data() :
                                         nullptr;
    stage.color_blend->ci.blendConstants[0] = 0.f;
    stage.color_blend->ci.blendConstants[1] = 0.f;
    stage.color_blend->ci.blendConstants[2] = 0.f;
    stage.color_blend->ci.blendConstants[3] = 0.f;
}

PipelineBuilder::PipelineBuilder(PipelineBuilder const& rhs)
    :stage(rhs.stage), m_device(rhs.m_device)
{
    stage.refreshReferences();
}

PipelineBuilder& PipelineBuilder::operator=(PipelineBuilder const& rhs)
{
    if (&rhs != this) {
        stage = rhs.stage;
        m_device = rhs.m_device;
        stage.refreshReferences();
    }
    return *this;
}

void PipelineBuilder::addVertexBindings(VkVertexInputBindingDescription const* binding_data, uint32_t n_bindings,
                                        VkVertexInputAttributeDescription const* attributes_data, uint32_t n_attributes)
{
    GHULBUS_PRECONDITION(stage.vertex_input);
    stage.vertex_bindings.insert(stage.vertex_bindings.end(), binding_data, binding_data + n_bindings);
    stage.vertex_attributes.insert(stage.vertex_attributes.end(), attributes_data, attributes_data + n_attributes);
}

void PipelineBuilder::clearVertexBindings()
{
    stage.vertex_bindings.clear();
    stage.vertex_attributes.clear();
}

void PipelineBuilder::adjustViewportDimensions(uint32_t viewport_width, uint32_t viewport_height)
{
    stage.viewport->viewports.front().width = static_cast<float>(viewport_width);
    stage.viewport->viewports.front().height = static_cast<float>(viewport_height);
    stage.viewport->scissors.front().extent.width = viewport_width;
    stage.viewport->scissors.front().extent.height = viewport_height;
}

Pipeline PipelineBuilder::create(PipelineLayout& layout, VkPipelineShaderStageCreateInfo const* shader_stages,
                                 uint32_t shader_stages_size, VkRenderPass render_pass)
{
    if (stage.vertex_input) {
        stage.vertex_input->pVertexBindingDescriptions = (!stage.vertex_bindings.empty()) ?
                                                         stage.vertex_bindings.data() :
                                                         nullptr;
        stage.vertex_input->vertexBindingDescriptionCount = static_cast<uint32_t>(stage.vertex_bindings.size());
        stage.vertex_input->pVertexAttributeDescriptions = (!stage.vertex_attributes.empty()) ?
                                                           stage.vertex_attributes.data() :
                                                           nullptr;
        stage.vertex_input->vertexAttributeDescriptionCount = static_cast<uint32_t>(stage.vertex_attributes.size());
    }
    auto value_ptr = [](auto& opt) { return (opt.has_value()) ? (&opt.value()) : nullptr; };
    VkGraphicsPipelineCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.stageCount = shader_stages_size;
    create_info.pStages = shader_stages;
    create_info.pVertexInputState = value_ptr(stage.vertex_input);
    create_info.pInputAssemblyState = value_ptr(stage.input_assembly);
    create_info.pTessellationState = value_ptr(stage.tesselation);
    create_info.pViewportState = (stage.viewport.has_value()) ? (&stage.viewport->ci) : nullptr;
    create_info.pRasterizationState = &stage.rasterization;
    create_info.pMultisampleState = value_ptr(stage.multisample);
    create_info.pDepthStencilState = value_ptr(stage.depth_stencil);
    create_info.pColorBlendState = (stage.color_blend.has_value()) ? (&stage.color_blend->ci) : nullptr;
    create_info.pDynamicState = nullptr;
    create_info.layout = layout.getVkPipelineLayout();
    create_info.renderPass = render_pass;
    create_info.subpass = 0;
    create_info.basePipelineHandle = VK_NULL_HANDLE;
    create_info.basePipelineIndex = -1;

    VkPipeline pipeline;
    VkResult res = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    checkVulkanError(res, "Error in vkCreateGraphicsPipeline.");
    return Pipeline(m_device, pipeline);
}

}
