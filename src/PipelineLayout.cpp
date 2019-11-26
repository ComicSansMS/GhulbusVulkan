#include <gbVk/PipelineLayout.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

PipelineLayout::PipelineLayout(VkDevice logical_device, VkPipelineLayout pipeline_layout)
    :m_pipelineLayout(pipeline_layout), m_device(logical_device)
{
}

PipelineLayout::~PipelineLayout()
{
    if (m_pipelineLayout) {
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    }
}

PipelineLayout::PipelineLayout(PipelineLayout&& rhs)
    :m_pipelineLayout(rhs.m_pipelineLayout), m_device(rhs.m_device)
{
    rhs.m_pipelineLayout = nullptr;
    rhs.m_device = nullptr;
}

VkPipelineLayout PipelineLayout::getVkPipelineLayout()
{
    return m_pipelineLayout;
}
}
