#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_BUILDER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_PIPELINE_BUILDER_HPP

/** @file
*
* @brief Pipeline Builder.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <optional>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Pipeline;
class PipelineLayout;

class PipelineBuilder {
public:
    struct Stages {
        std::optional<VkPipelineVertexInputStateCreateInfo> vertex_input;
        std::vector<VkVertexInputBindingDescription> vertex_bindings;
        std::vector<VkVertexInputAttributeDescription> vertex_attributes;
        std::optional<VkPipelineInputAssemblyStateCreateInfo> input_assembly;
        std::optional<VkPipelineTessellationStateCreateInfo> tesselation;
        struct Viewport {
            std::vector<VkViewport> viewports;
            std::vector<VkRect2D> scissors;
            VkPipelineViewportStateCreateInfo ci;

            Viewport() = default;
            Viewport(Viewport const& rhs);
            Viewport& operator=(Viewport const& rhs);
            Viewport(Viewport&& rhs);
            Viewport& operator==(Viewport&& rhs);
            void refreshReferences();
        };
        std::optional<Viewport> viewport;
        VkPipelineRasterizationStateCreateInfo rasterization;
        std::optional<VkPipelineMultisampleStateCreateInfo> multisample;
        std::optional<VkPipelineDepthStencilStateCreateInfo> depth_stencil;
        struct ColorBlend {
            std::vector<VkPipelineColorBlendAttachmentState> attachments;
            VkPipelineColorBlendStateCreateInfo ci;

            ColorBlend() = default;
            ColorBlend(ColorBlend const& rhs);
            ColorBlend& operator=(ColorBlend const& rhs);
            ColorBlend(ColorBlend&& rhs);
            ColorBlend& operator=(ColorBlend&& rhs);
            void refreshReferences();
        };
        std::optional<ColorBlend> color_blend;
    } stage;

private:
    VkDevice m_device;
public:

    PipelineBuilder(VkDevice logical_device, uint32_t viewport_width, uint32_t viewport_height);

    void addVertexBindings(VkVertexInputBindingDescription* binding_data, uint32_t n_bindings,
                           VkVertexInputAttributeDescription* attributes_data, uint32_t n_attributes);

    Pipeline create(PipelineLayout& layout, VkPipelineShaderStageCreateInfo const* shader_stages,
                    uint32_t shader_stages_size, VkRenderPass render_pass);
};
}
#endif
