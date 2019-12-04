#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SET_LAYOUT_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SET_LAYOUT_HPP

/** @file
*
* @brief Descriptor Set Layout.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

class DescriptorSetLayout {
private:
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDevice m_device;
public:
    DescriptorSetLayout(VkDevice logical_device, VkDescriptorSetLayout descriptor_set_layout);

    ~DescriptorSetLayout();

    DescriptorSetLayout(DescriptorSetLayout const&) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout const&) = delete;

    DescriptorSetLayout(DescriptorSetLayout&& rhs);
    DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

    VkDescriptorSetLayout getVkDescriptorSetLayout() const;
};
}
#endif
