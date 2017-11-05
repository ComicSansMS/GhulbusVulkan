#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SHADER_MODULE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SHADER_MODULE_HPP

/** @file
*
* @brief Shader module.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

class ShaderModule {
private:
    VkShaderModule m_shaderModule;
    VkDevice m_device;
public:
    ShaderModule(VkDevice logical_device, VkShaderModule shader_module);

    ~ShaderModule();

    ShaderModule(ShaderModule const&) = delete;
    ShaderModule& operator=(ShaderModule const&) = delete;

    ShaderModule(ShaderModule&& rhs);
    ShaderModule& operator=(ShaderModule&&) = delete;

    VkShaderModule getVkShaderModule();
};
}
#endif
