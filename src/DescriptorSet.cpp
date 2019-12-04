#include <gbVk/DescriptorSet.hpp>

#include <gbVk/Exceptions.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DescriptorSet::DescriptorSet(VkDescriptorSet descriptor_set)
    :m_descriptorSet(descriptor_set)
{
}

DescriptorSet::~DescriptorSet()
{}

DescriptorSet::DescriptorSet(DescriptorSet&& rhs)
    :m_descriptorSet(rhs.m_descriptorSet)
{
    rhs.m_descriptorSet = nullptr;
}

VkDescriptorSet DescriptorSet::getVkDescriptorSet()
{
    return m_descriptorSet;
}
}
