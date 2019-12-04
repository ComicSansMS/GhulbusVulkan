#include <gbVk/DescriptorPoolBuilder.hpp>

#include <gbVk/DescriptorPool.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DescriptorPoolBuilder::DescriptorPoolBuilder(VkDevice logical_device)
    :m_device(logical_device)
{
}

void DescriptorPoolBuilder::addDescriptorPoolSize(VkDescriptorType type, uint32_t size)
{
    descriptorPoolSizes.push_back(VkDescriptorPoolSize{ type, size });
}

DescriptorPool DescriptorPoolBuilder::create(uint32_t max_sets, VkDescriptorPoolCreateFlags flags)
{
    return create(max_sets, flags | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, NoImplicitFreeDescriptorFlag{});
}

DescriptorPool DescriptorPoolBuilder::create(uint32_t max_sets, VkDescriptorPoolCreateFlags flags,
                                             NoImplicitFreeDescriptorFlag const&)
{
    GHULBUS_PRECONDITION(max_sets > 0);
    VkDescriptorPoolCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.pNext = nullptr;
    create_info.flags = flags;
    create_info.maxSets = max_sets;
    create_info.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    create_info.pPoolSizes = descriptorPoolSizes.data();
    VkDescriptorPool descriptor_pool;
    VkResult const res = vkCreateDescriptorPool(m_device, &create_info, nullptr, &descriptor_pool);
    checkVulkanError(res, "Error in vkCreateDescriptorPool.");
    return DescriptorPool(m_device, descriptor_pool,
        ((flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) != 0));
}
}
