#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SET_LAYOUT_BUILDER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_SET_LAYOUT_BUILDER_HPP

/** @file
*
* @brief Descriptor Set Layout Builder.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DescriptorSetLayout;

class DescriptorSetLayoutBuilder {
public:
    std::vector<VkDescriptorSetLayoutBinding> bindings;

private:
    VkDevice m_device;
public:
    DescriptorSetLayoutBuilder(VkDevice logical_device);

    void addUniformBuffer(uint32_t binding, VkShaderStageFlags flags);

    void addSampler(uint32_t binding, VkShaderStageFlags flags);

    DescriptorSetLayout create();
};
}
#endif
