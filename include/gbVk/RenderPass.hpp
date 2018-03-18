#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_RENDER_PASS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_RENDER_PASS_HPP

/** @file
*
* @brief Render pass.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class RenderPass {
private:
    VkRenderPass m_renderPass;
    VkDevice m_device;

public:
    RenderPass(VkDevice logical_device, VkRenderPass render_pass);

    ~RenderPass();

    RenderPass(RenderPass const&) = delete;
    RenderPass& operator=(RenderPass const&) = delete;

    RenderPass(RenderPass&& rhs);
    RenderPass& operator=(RenderPass&&) = delete;

    VkRenderPass getVkRenderPass();
};
}
#endif
