#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SET_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SET_HPP

/** @file
*
* @brief Descriptor Set.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DescriptorSet {
private:
    VkDescriptorSet m_descriptorSet;
public:
    explicit DescriptorSet(VkDescriptorSet descriptor_set);
    ~DescriptorSet();

    DescriptorSet(DescriptorSet const&) = delete;
    DescriptorSet& operator=(DescriptorSet const&) = delete;

    DescriptorSet(DescriptorSet&& rhs);
    DescriptorSet& operator=(DescriptorSet&& rhs) = delete;

    VkDescriptorSet getVkDescriptorSet();
};
}
#endif
