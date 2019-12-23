#include <gbGraphics/Program.hpp>

#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/VertexFormat.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/PipelineBuilder.hpp>
#include <gbVk/SpirvCode.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <spirv_cross.hpp>

#include <algorithm>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
struct Program::ReflectionInfo {
    spirv_cross::Compiler vertex;
    spirv_cross::Compiler fragment;

    spirv_cross::SmallVector<spirv_cross::EntryPoint> vertex_entry_points;
    spirv_cross::SmallVector<spirv_cross::EntryPoint> fragment_entry_points;

    spirv_cross::ShaderResources vertex_resources;
    spirv_cross::ShaderResources fragment_resources;

    ReflectionInfo(GhulbusVulkan::SpirvCode const& vertex_shader, GhulbusVulkan::SpirvCode const& fragment_shader);

    ReflectionInfo(ReflectionInfo const&) = delete;
    ReflectionInfo& operator=(ReflectionInfo const&) = delete;
};

Program::ReflectionInfo::ReflectionInfo(GhulbusVulkan::SpirvCode const& vertex_shader,
                                        GhulbusVulkan::SpirvCode const& fragment_shader)
    :vertex(vertex_shader.getCodeAsStdVector()), fragment(fragment_shader.getCodeAsStdVector()),
     vertex_entry_points(vertex.get_entry_points_and_stages()),
     fragment_entry_points(fragment.get_entry_points_and_stages()),
     vertex_resources(vertex.get_shader_resources()), fragment_resources(fragment.get_shader_resources())
{
    if (vertex_entry_points.empty() || vertex_entry_points[0].execution_model != spv::ExecutionModelVertex) {
        GHULBUS_THROW(Exceptions::ShaderError(), "Not a vertex shader.");
    }
    GHULBUS_PRECONDITION_MESSAGE(vertex_entry_points.size() == 1, "Multiple entry points currently not supported.");
    if (fragment_entry_points.empty() || fragment_entry_points[0].execution_model != spv::ExecutionModelFragment) {
        GHULBUS_THROW(Exceptions::ShaderError(), "Not a fragment shader.");
    }
    GHULBUS_PRECONDITION_MESSAGE(fragment_entry_points.size() == 1, "Multiple entry points currently not supported.");

    for (spirv_cross::Resource const& r : vertex_resources.stage_inputs) {
        uint32_t const deco_set = vertex.get_decoration(r.id, spv::DecorationDescriptorSet);
        uint32_t const deco_binding = vertex.get_decoration(r.id, spv::DecorationBinding);
        uint32_t const deco_attachment = vertex.get_decoration(r.id, spv::DecorationInputAttachmentIndex);
        uint32_t const deco_location = vertex.get_decoration(r.id, spv::DecorationLocation);
        GHULBUS_LOG(Info, "Vertex Input Descriptor Set#" << deco_set << " Binding#" << deco_binding << " Attachment#" << deco_attachment << " Location#" << deco_location << " " << vertex.get_name(r.base_type_id));
        spirv_cross::SPIRType const rbtype = vertex.get_type(r.base_type_id);
        spirv_cross::SPIRType const rtype = vertex.get_type(r.type_id);
    }
}

Program::Program(GraphicsInstance& instance, GhulbusVulkan::SpirvCode const& vertex_shader,
                 GhulbusVulkan::SpirvCode const& fragment_shader)
    :m_instance(&instance), m_vertexShader(instance.getVulkanDevice().createShaderModule(vertex_shader)),
     m_fragmentShader(instance.getVulkanDevice().createShaderModule(fragment_shader)),
     m_reflection(std::make_unique<ReflectionInfo>(vertex_shader, fragment_shader))
{
    m_shaderStageCreateInfos = createShaderStageCreateInfos();
}

Program::~Program() = default;

VkPipelineShaderStageCreateInfo const* Program::getShaderStageCreateInfos() const
{
    GHULBUS_PRECONDITION(!m_shaderStageCreateInfos.empty());
    return m_shaderStageCreateInfos.data();
}

uint32_t Program::getNumberOfShaderStages() const
{
    return static_cast<uint32_t>(m_shaderStageCreateInfos.size());
}

std::vector<VkPipelineShaderStageCreateInfo> Program::createShaderStageCreateInfos()
{
    std::vector<VkPipelineShaderStageCreateInfo> ret;
    ret.resize(2);

    VkPipelineShaderStageCreateInfo& vert_shader_stage_ci = ret[0];
    vert_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_ci.pNext = nullptr;
    vert_shader_stage_ci.flags = 0;
    vert_shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_ci.module = m_vertexShader.getVkShaderModule();
    vert_shader_stage_ci.pName = m_reflection->vertex_entry_points[0].name.c_str();
    vert_shader_stage_ci.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo& frag_shader_stage_ci = ret[1];
    frag_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_ci.pNext = nullptr;
    frag_shader_stage_ci.flags = 0;
    frag_shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_ci.module = m_fragmentShader.getVkShaderModule();
    frag_shader_stage_ci.pName = m_reflection->fragment_entry_points[0].name.c_str();
    frag_shader_stage_ci.pSpecializationInfo = nullptr;

    return ret;
}

namespace {
VkFormat determineAttributeFormat(VertexFormatBase::ComponentType type, VertexFormatBase::ComponentSemantics semantics)
{
    GHULBUS_UNUSED_VARIABLE(semantics);
    switch (type)
    {
    case VertexFormatBase::ComponentType::t_vec2:
        return VK_FORMAT_R32G32_SFLOAT;
    case VertexFormatBase::ComponentType::t_vec3:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case VertexFormatBase::ComponentType::t_vec4:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    default:
        GHULBUS_THROW(Exceptions::NotImplemented(), "Attribute detection for type not implemented.");
    }
}

VertexFormatBase::ComponentType getMatchingComponentType(spirv_cross::SPIRType const& spir_type)
{
    switch(spir_type.basetype) {
    case spirv_cross::SPIRType::Float:
        if(spir_type.columns == 1) {
            if(spir_type.vecsize == 2) {
                return VertexFormatBase::ComponentType::t_vec2;
            } else if(spir_type.vecsize == 3) {
                return VertexFormatBase::ComponentType::t_vec3;
            } else if(spir_type.vecsize == 4) {
                return VertexFormatBase::ComponentType::t_vec4;
            }
        }
        break;
    default:
        break;
    }
    GHULBUS_THROW(Exceptions::NotImplemented(), "Spir type not supported.");
}

void checkTypeMatch(spirv_cross::SPIRType const& spir_type, VertexFormatBase::ComponentType component_type)
{
    if (getMatchingComponentType(spir_type) != component_type) {
        GHULBUS_THROW(Exceptions::ShaderError(), "Vertex Attribute type mismatch.");
    }
}
}

void Program::addVertexBinding(uint32_t binding, VertexFormatBase const& vertex_input_format)
{
    m_vertexInputFormat.emplace_back(vertex_input_format.clone());
    VkVertexInputBindingDescription vertex_binding;
    vertex_binding.binding = binding;
    vertex_binding.stride = static_cast<uint32_t>(vertex_input_format.getStride());
    vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    m_vertexInputBinding.push_back(vertex_binding);
}

void Program::bindVertexInput(VertexFormatBase::ComponentSemantics component_semantic,
                              uint32_t binding, uint32_t location)
{
    auto const it = std::find_if(m_vertexInputBinding.begin(), m_vertexInputBinding.end(),
        [binding](VkVertexInputBindingDescription const& vertex_binding) { return vertex_binding.binding == binding; });
    GHULBUS_PRECONDITION(it != m_vertexInputBinding.end());
    auto const binding_index = std::distance(m_vertexInputBinding.begin(), it);
    VertexFormatBase& format = *m_vertexInputFormat[binding_index];
    size_t const component_index = [&format](VertexFormatBase::ComponentSemantics s) {
        for (std::size_t i = 0; i < format.getNumberOfComponents(); ++i) {
            if (format.getComponentSemantics(i) == s) { return i; }
        }
        GHULBUS_THROW(Exceptions::ShaderError(), "Requested semantic is not part of vertex format.");
    }(component_semantic);

    auto const& resources = m_reflection->vertex_resources.stage_inputs;
    auto const it_resource = 
        std::find_if(resources.begin(), resources.end(), [this, binding, location](spirv_cross::Resource const& r) {
            uint32_t const rbinding = m_reflection->vertex.get_decoration(r.id, spv::DecorationBinding);
            uint32_t const rlocation = m_reflection->vertex.get_decoration(r.id, spv::DecorationLocation);
            return (rbinding == binding) && (rlocation == location);
        });
    if(it_resource == resources.end()) {
        GHULBUS_THROW(Exceptions::ShaderError(), "Invalid binding/location indices.");
    }
    checkTypeMatch(m_reflection->vertex.get_type(it_resource->base_type_id), format.getComponentType(component_index));

    VkVertexInputAttributeDescription vertex_attribute;
    vertex_attribute.location = location;
    vertex_attribute.binding = binding;
    vertex_attribute.format = determineAttributeFormat(format.getComponentType(component_index),
                                                       format.getComponentSemantics(component_index));
    vertex_attribute.offset = static_cast<uint32_t>(format.getComponentOffset(component_index));

    m_vertexInputAttributes.push_back(vertex_attribute);
}

std::vector<VkVertexInputBindingDescription> const& Program::getVertexInputBindings() const
{
    return m_vertexInputBinding;
}

std::vector<VkVertexInputAttributeDescription> const& Program::getVertexInputAttributeDescriptions() const
{
    return m_vertexInputAttributes;
}
}
