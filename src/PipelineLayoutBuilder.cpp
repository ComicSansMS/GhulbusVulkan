#include <gbVk/PipelineLayoutBuilder.hpp>

#include <gbVk/DescriptorSetLayout.hpp>
#include <gbVk/PipelineLayout.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
PipelineLayoutBuilder::PipelineLayoutBuilder(VkDevice logical_device)
    :m_device(logical_device)
{}

void PipelineLayoutBuilder::addDescriptorSetLayout(DescriptorSetLayout& descriptor_set_layout)
{
    descriptor_set_layouts.emplace_back(descriptor_set_layout.getVkDescriptorSetLayout());
}

PipelineLayout PipelineLayoutBuilder::create()
{
    VkPipelineLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    create_info.pSetLayouts = (!descriptor_set_layouts.empty()) ? descriptor_set_layouts.data() : nullptr;
    create_info.pushConstantRangeCount = 0;
    create_info.pPushConstantRanges = nullptr;
    VkPipelineLayout pipeline_layout;
    VkResult res = vkCreatePipelineLayout(m_device, &create_info, nullptr, &pipeline_layout);
    checkVulkanError(res, "Error in vkCreatePipelineLayout.");
    return PipelineLayout(m_device, pipeline_layout);
}
}
