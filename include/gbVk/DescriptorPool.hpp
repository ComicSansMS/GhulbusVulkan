#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_POOL_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_POOL_HPP

/** @file
*
* @brief Descriptor Pool.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DescriptorSets;
class DescriptorSetLayout;

class DescriptorPool {
private:
    VkDescriptorPool m_descriptorPool;
    VkDevice m_device;
    bool m_setsHaveOwnership;
public:
    DescriptorPool(VkDevice logical_device, VkDescriptorPool descriptor_pool, bool allocated_sets_have_ownership);
    ~DescriptorPool();

    DescriptorPool(DescriptorPool const&) = delete;
    DescriptorPool& operator=(DescriptorPool const&) = delete;

    DescriptorPool(DescriptorPool&& rhs);
    DescriptorPool& operator=(DescriptorPool&& rhs) = delete;

    DescriptorSets allocateDescriptorSets(uint32_t descriptor_set_count, DescriptorSetLayout& layout);
    DescriptorSets allocateDescriptorSets(uint32_t descriptor_set_count,
                                          std::vector<DescriptorSetLayout> const& layouts);
    DescriptorSets allocateDescriptorSets(uint32_t descriptor_set_count, VkDescriptorSetLayout* layouts);
};
}
#endif
