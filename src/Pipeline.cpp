#include <gbVk/Pipeline.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

Pipeline::Pipeline(VkDevice logical_device, VkPipeline pipeline)
    :m_pipeline(pipeline), m_device(logical_device)
{
}

Pipeline::Pipeline(Pipeline&& rhs)
    :m_pipeline(rhs.m_pipeline), m_device(rhs.m_device)
{
    rhs.m_pipeline = nullptr;
}

Pipeline::~Pipeline()
{
    if (m_pipeline) {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
    }
}

VkPipeline Pipeline::getVkPipeline()
{
    return m_pipeline;
}

}


