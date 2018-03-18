#include <gbVk/RenderPass.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
RenderPass::RenderPass(VkDevice logical_device, VkRenderPass render_pass)
    :m_renderPass(render_pass), m_device(logical_device)
{}

RenderPass::~RenderPass()
{
    if(m_renderPass) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    }
}

RenderPass::RenderPass(RenderPass&& rhs)
    :m_renderPass(rhs.m_renderPass), m_device(rhs.m_device)
{
    rhs.m_renderPass = nullptr;
    rhs.m_device = nullptr;
}

VkRenderPass RenderPass::getVkRenderPass()
{
    return m_renderPass;
}
}
