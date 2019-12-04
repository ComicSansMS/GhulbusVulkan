#include <gbVk/DescriptorPool.hpp>

#include <gbVk/DescriptorSets.hpp>
#include <gbVk/DescriptorSetLayout.hpp>
#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

#include <algorithm>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
DescriptorPool::DescriptorPool(VkDevice logical_device, VkDescriptorPool descriptor_pool,
                               bool allocated_sets_have_ownership)
    :m_descriptorPool(descriptor_pool), m_device(logical_device), m_setsHaveOwnership(allocated_sets_have_ownership)
{
}

DescriptorPool::DescriptorPool(DescriptorPool&& rhs)
    :m_descriptorPool(rhs.m_descriptorPool), m_device(rhs.m_device), m_setsHaveOwnership(rhs.m_setsHaveOwnership)
{
    rhs.m_descriptorPool = nullptr;
}

DescriptorPool::~DescriptorPool()
{
    if (m_descriptorPool) {
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    }
}

DescriptorSets DescriptorPool::allocateDescriptorSets(uint32_t descriptor_set_count, DescriptorSetLayout& layout)
{
    std::vector<VkDescriptorSetLayout> vklayouts(descriptor_set_count, layout.getVkDescriptorSetLayout());
    return allocateDescriptorSets(descriptor_set_count, vklayouts.data());
}

DescriptorSets DescriptorPool::allocateDescriptorSets(uint32_t descriptor_set_count,
                                                      std::vector<DescriptorSetLayout> const& layouts)
{
    GHULBUS_PRECONDITION(static_cast<uint32_t>(layouts.size()) == descriptor_set_count);
    std::vector<VkDescriptorSetLayout> vklayouts(descriptor_set_count);
    std::transform(begin(layouts), end(layouts), begin(vklayouts),
                   [](DescriptorSetLayout const& l) { return l.getVkDescriptorSetLayout(); });
    return allocateDescriptorSets(descriptor_set_count, vklayouts.data());
}

DescriptorSets DescriptorPool::allocateDescriptorSets(uint32_t descriptor_set_count, VkDescriptorSetLayout* layouts)
{
    VkDescriptorSetAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = nullptr;
    alloc_info.descriptorPool = m_descriptorPool;
    alloc_info.descriptorSetCount = descriptor_set_count;
    alloc_info.pSetLayouts = layouts;
    std::vector<VkDescriptorSet> descriptor_sets;
    descriptor_sets.resize(descriptor_set_count);
    VkResult const res = vkAllocateDescriptorSets(m_device, &alloc_info, descriptor_sets.data());
    checkVulkanError(res, "Error in vkAllocateDescriptorSets.");
    return (m_setsHaveOwnership) ? DescriptorSets(m_device, m_descriptorPool, std::move(descriptor_sets)) :
        DescriptorSets(m_device, m_descriptorPool, std::move(descriptor_sets), DescriptorSets::NonOwning{});
}
}
