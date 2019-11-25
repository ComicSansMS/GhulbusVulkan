#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_HPP

/** @file
*
* @brief Pipeline.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

class Pipeline {
private:
    VkPipeline m_pipeline;
    VkDevice m_device;
public:
    explicit Pipeline(VkDevice logical_device, VkPipeline pipeline);

    ~Pipeline();

    Pipeline(Pipeline const&) = delete;
    Pipeline& operator=(Pipeline const&) = delete;

    Pipeline(Pipeline&& rhs);
    Pipeline& operator=(Pipeline&&) = delete;

    VkPipeline getVkPipeline();

};
}
#endif
