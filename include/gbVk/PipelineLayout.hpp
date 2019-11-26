#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_LAYOUT_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_LAYOUT_HPP

/** @file
*
* @brief Pipeline Layout.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class PipelineLayout {
private:
    VkPipelineLayout m_pipelineLayout;
    VkDevice m_device;
public:
    PipelineLayout(VkDevice logical_device, VkPipelineLayout pipeline_layout);

    ~PipelineLayout();

    PipelineLayout(PipelineLayout const&) = delete;
    PipelineLayout& operator=(PipelineLayout const&) = delete;

    PipelineLayout(PipelineLayout&& rhs);
    PipelineLayout& operator=(PipelineLayout&& rhs) = delete;

    VkPipelineLayout getVkPipelineLayout();
};
}
#endif
