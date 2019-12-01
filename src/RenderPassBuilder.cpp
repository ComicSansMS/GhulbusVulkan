#include <gbVk/RenderPassBuilder.hpp>

#include <gbVk/Exceptions.hpp>
#include <gbVk/RenderPass.hpp>

#include <gbBase/Assert.hpp>

#include <limits>

namespace GHULBUS_VULKAN_NAMESPACE
{
RenderPassBuilder::RenderPassBuilder(VkDevice logical_device)
    :m_device(logical_device)
{}

void RenderPassBuilder::addSubpassGraphics()
{
    VkSubpassDescription subpass_desc;
    subpass_desc.flags = 0;
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.inputAttachmentCount = 0;
    subpass_desc.pInputAttachments = nullptr;
    subpass_desc.colorAttachmentCount = 0;
    subpass_desc.pColorAttachments = nullptr;
    subpass_desc.pResolveAttachments = nullptr;
    subpass_desc.pDepthStencilAttachment = nullptr;
    subpass_desc.preserveAttachmentCount = 0;
    subpass_desc.pPreserveAttachments = nullptr;
    subpassDescriptions.push_back(subpass_desc);
    attachmentReferences.push_back(std::vector<VkAttachmentReference>());

    VkSubpassDependency subpass_dep;
    subpass_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dep.dstSubpass = 0;
    subpass_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep.srcAccessMask = 0;
    subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dep.dependencyFlags = 0;
    subpassDependencies.push_back(subpass_dep);
}

void RenderPassBuilder::addColorAttachment(VkFormat image_format)
{
    GHULBUS_PRECONDITION(!attachmentReferences.empty());
    GHULBUS_PRECONDITION(attachmentReferences.size() < std::numeric_limits<uint32_t>::max());
    GHULBUS_PRECONDITION(!subpassDescriptions.empty());

    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments.push_back(color_attachment);

    VkAttachmentReference color_attachment_ref;
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentReferences.back().push_back(color_attachment_ref);

    subpassDescriptions.back().colorAttachmentCount = static_cast<uint32_t>(attachmentReferences.back().size());
    subpassDescriptions.back().pColorAttachments = attachmentReferences.back().data();
}

RenderPass RenderPassBuilder::create()
{
    GHULBUS_PRECONDITION(!subpassDescriptions.empty());
    GHULBUS_PRECONDITION(attachments.size() < std::numeric_limits<uint32_t>::max());
    GHULBUS_PRECONDITION(subpassDescriptions.size() < std::numeric_limits<uint32_t>::max());
    GHULBUS_PRECONDITION(subpassDependencies.size() < std::numeric_limits<uint32_t>::max());

    VkRenderPassCreateInfo render_pass_ci;
    render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_ci.pNext = nullptr;
    render_pass_ci.flags = 0;
    render_pass_ci.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_ci.pAttachments = (!attachments.empty()) ? attachments.data() : nullptr;
    render_pass_ci.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
    render_pass_ci.pSubpasses = (!subpassDescriptions.empty()) ? subpassDescriptions.data() : nullptr;
    render_pass_ci.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
    render_pass_ci.pDependencies = (!subpassDependencies.empty()) ? subpassDependencies.data() : nullptr;

    VkRenderPass render_pass;
    VkResult res = vkCreateRenderPass(m_device, &render_pass_ci, nullptr, &render_pass);
    checkVulkanError(res, "Error in vkCreateRenderPass.");
    return RenderPass(m_device, render_pass);
}
}
