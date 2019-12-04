#include <gbVk/DescriptorSets.hpp>

#include <gbVk/DescriptorSet.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DescriptorSets::DescriptorSets(VkDevice logical_device, VkDescriptorPool descriptor_pool,
                               std::vector<VkDescriptorSet> descriptor_sets)
    :m_descriptorSets(std::move(descriptor_sets)), m_device(logical_device), m_descriptorPool(descriptor_pool),
     m_hasOwnership(true)
{
}

DescriptorSets::DescriptorSets(VkDevice logical_device, VkDescriptorPool descriptor_pool,
                               std::vector<VkDescriptorSet> descriptor_sets, NonOwning const&)
    :DescriptorSets(logical_device, descriptor_pool, std::move(descriptor_sets))
{
    m_hasOwnership = false;
}

DescriptorSets::~DescriptorSets()
{
    if (m_hasOwnership && !m_descriptorSets.empty()) {
        vkFreeDescriptorSets(m_device, m_descriptorPool,
                             static_cast<uint32_t>(m_descriptorSets.size()), m_descriptorSets.data());
    }
}

DescriptorSets::DescriptorSets(DescriptorSets&& rhs)
    :m_descriptorSets(std::move(rhs.m_descriptorSets)), m_device(rhs.m_device), m_descriptorPool(rhs.m_descriptorPool),
     m_hasOwnership(rhs.m_hasOwnership)
{
    rhs.m_descriptorSets.clear();
}

uint32_t DescriptorSets::size() const
{
    return static_cast<uint32_t>(m_descriptorSets.size());
}

DescriptorSet DescriptorSets::getDescriptorSet(uint32_t index)
{
    GHULBUS_PRECONDITION_DBG((index >= 0) && (index < size()));
    return DescriptorSet(m_descriptorSets[index]);
}

VkDescriptorSet* DescriptorSets::getVkDescriptorSets()
{
    return m_descriptorSets.data();
}
}
