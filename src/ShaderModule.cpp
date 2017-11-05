#include <gbVk/ShaderModule.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
ShaderModule::ShaderModule(VkDevice logical_device, VkShaderModule shader_module)
    :m_shaderModule(shader_module), m_device(logical_device)
{
}

ShaderModule::~ShaderModule()
{
    if(m_shaderModule) { vkDestroyShaderModule(m_device, m_shaderModule, nullptr); }
}

ShaderModule::ShaderModule(ShaderModule&& rhs)
    :m_shaderModule(rhs.m_shaderModule), m_device(rhs.m_device)
{
    rhs.m_shaderModule = nullptr;
    rhs.m_device = nullptr;
}

VkShaderModule ShaderModule::getVkShaderModule()
{
    return m_shaderModule;
}
}
