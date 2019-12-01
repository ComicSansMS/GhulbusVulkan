#include <gbVk/DescriptorSetLayout.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DescriptorSetLayout::DescriptorSetLayout(VkDevice logical_device, VkDescriptorSetLayout descriptor_set_layout)
    :m_descriptorSetLayout(descriptor_set_layout), m_device(logical_device)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    if (m_descriptorSetLayout) {
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    }
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& rhs)
    :m_descriptorSetLayout(rhs.m_descriptorSetLayout), m_device(rhs.m_device)
{
    rhs.m_descriptorSetLayout = nullptr;
    rhs.m_device = nullptr;
}

VkDescriptorSetLayout DescriptorSetLayout::getVkDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}
}
