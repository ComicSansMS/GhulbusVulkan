#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SETS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SETS_HPP

/** @file
*
* @brief Descriptor Sets.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DescriptorSet;

class DescriptorSets {
public:
    struct NonOwning {};
private:
    std::vector<VkDescriptorSet> m_descriptorSets;
    VkDevice m_device;
    VkDescriptorPool m_descriptorPool;
    bool m_hasOwnership;
public:
    DescriptorSets(VkDevice logical_device, VkDescriptorPool descriptor_pool,
                   std::vector<VkDescriptorSet> descriptor_sets);

    DescriptorSets(VkDevice logical_device, VkDescriptorPool descriptor_pool,
                   std::vector<VkDescriptorSet> descriptor_sets, NonOwning const&);

    ~DescriptorSets();

    DescriptorSets(DescriptorSets const&) = delete;
    DescriptorSets& operator=(DescriptorSets const&) = delete;

    DescriptorSets(DescriptorSets&& rhs);
    DescriptorSets& operator=(DescriptorSets&&) = delete;

    uint32_t size() const;

    DescriptorSet getDescriptorSet(uint32_t index);

    VkDescriptorSet* getVkDescriptorSets();
};
}
#endif
