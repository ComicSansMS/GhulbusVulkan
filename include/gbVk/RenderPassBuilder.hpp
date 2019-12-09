#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_RENDER_PASS_BUILDER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_RENDER_PASS_BUILDER_HPP

/** @file
*
* @brief Render pass Builder.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <optional>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class RenderPass;

class RenderPassBuilder {
public:
    std::vector<VkAttachmentDescription> attachments;
    std::vector<std::vector<VkAttachmentReference>> attachmentReferences;
    std::optional<VkAttachmentReference> attachmentReferenceDepthStencil;
    std::vector<VkSubpassDescription> subpassDescriptions;
    std::vector<VkSubpassDependency> subpassDependencies;
private:
    VkDevice m_device;
public:
    RenderPassBuilder(VkDevice logical_device);

    ~RenderPassBuilder() = default;
    RenderPassBuilder(RenderPassBuilder const&) = default;
    RenderPassBuilder& operator=(RenderPassBuilder const&) = default;

    void addSubpassGraphics();

    void addColorAttachment(VkFormat image_format);

    void addDepthStencilAttachment(VkFormat depth_format);

    RenderPass create();
};
}
#endif
