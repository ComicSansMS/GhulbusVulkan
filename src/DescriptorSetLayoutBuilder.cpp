#include <gbVk/DescriptorSetLayoutBuilder.hpp>

#include <gbVk/DescriptorSetLayout.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(VkDevice logical_device)
    :m_device(logical_device)
{}

void DescriptorSetLayoutBuilder::addUniformBuffer(uint32_t binding, VkShaderStageFlags flags)
{
    bindings.emplace_back();
    VkDescriptorSetLayoutBinding& ubo_layout_binding = bindings.back();
    ubo_layout_binding.binding = binding;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = flags;
    ubo_layout_binding.pImmutableSamplers = nullptr;
}

DescriptorSetLayout DescriptorSetLayoutBuilder::create()
{
    VkDescriptorSetLayoutCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    create_info.pBindings = bindings.data();

    VkDescriptorSetLayout desc_set_layout;
    VkResult res = vkCreateDescriptorSetLayout(m_device, &create_info, nullptr, &desc_set_layout);
    checkVulkanError(res, "Error in vkCreateDescriptorSetLayout.");
    return DescriptorSetLayout(m_device, desc_set_layout);
}
}
