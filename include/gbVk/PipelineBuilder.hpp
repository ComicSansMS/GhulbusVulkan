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

namespace GHULBUS_VULKAN_NAMESPACE
{

class PipelineBuilder {
public:
    struct Stages {
        std::optional<VkPipelineVertexInputStateCreateInfo> vertex_input;
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

    static PipelineBuilder graphicsPipeline(int viewport_width, int viewport_height);
};
}
#endif
