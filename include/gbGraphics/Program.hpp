#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_PROGRAM_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_PROGRAM_HPP

/** @file
*
* @brief Program.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/VertexFormat.hpp>

#include <gbVk/ForwardDecl.hpp>
#include <gbVk/ShaderModule.hpp>

#include <vector>
#include <memory>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;

class Program {
private:
    GhulbusGraphics::GraphicsInstance* m_instance;
    GhulbusVulkan::ShaderModule m_vertexShader;
    GhulbusVulkan::ShaderModule m_fragmentShader;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageCreateInfos;
    struct ReflectionInfo;
    std::unique_ptr<ReflectionInfo> m_reflection;

    std::vector<std::unique_ptr<VertexFormatBase>> m_vertexInputFormat;
    std::vector<VkVertexInputBindingDescription> m_vertexInputBinding;
    std::vector<VkVertexInputAttributeDescription> m_vertexInputAttributes;
public:
    Program(GraphicsInstance& instance, GhulbusVulkan::SpirvCode const& vertex_shader,
            GhulbusVulkan::SpirvCode const& fragment_shader);

    ~Program();

    VkPipelineShaderStageCreateInfo const* getShaderStageCreateInfos() const;
    uint32_t getNumberOfShaderStages() const;

    void addVertexBinding(uint32_t binding, VertexFormatBase const& vertex_input_format);
    void bindVertexInput(VertexFormatBase::ComponentSemantics component_semantic, uint32_t binding, uint32_t location);

    std::vector<VkVertexInputBindingDescription> const& getVertexInputBindings() const;
    std::vector<VkVertexInputAttributeDescription> const& getVertexInputAttributeDescriptions() const;
private:
    std::vector<VkPipelineShaderStageCreateInfo> createShaderStageCreateInfos();
};
}
#endif
