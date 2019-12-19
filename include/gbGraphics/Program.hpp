#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_PROGRAM_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_PROGRAM_HPP

/** @file
*
* @brief Program.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>
#include <gbVk/ShaderModule.hpp>

#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;

class Program {
private:
    GhulbusGraphics::GraphicsInstance* m_instance;
    GhulbusVulkan::ShaderModule m_vertexShader;
    GhulbusVulkan::ShaderModule m_fragmentShader;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStageCreateInfos;
public:
    Program(GraphicsInstance& instance, GhulbusVulkan::SpirvCode const& vertex_shader,
            GhulbusVulkan::SpirvCode const& fragment_shader);

    VkPipelineShaderStageCreateInfo const* getShaderStageCreateInfos() const;
    uint32_t getNumberOfShaderStages() const;
private:
    std::vector<VkPipelineShaderStageCreateInfo> createShaderStageCreateInfos();
};
}
#endif
