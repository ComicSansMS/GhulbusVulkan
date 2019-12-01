#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_LAYOUT_BUILDER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_LAYOUT_BUILDER_HPP

/** @file
*
* @brief Pipeline Layout Builder.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DescriptorSetLayout;
class PipelineLayout;

class PipelineLayoutBuilder {
public:
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;

private:
    VkDevice m_device;
public:
    PipelineLayoutBuilder(VkDevice logical_device);

    void addDescriptorSetLayout(DescriptorSetLayout& descriptor_set_layout);

    PipelineLayout create();
};
}
#endif
