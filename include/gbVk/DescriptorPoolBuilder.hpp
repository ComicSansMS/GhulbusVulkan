#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_POOL_BUILDER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DESCRIPTOR_POOL_BUILDER_HPP

/** @file
*
* @brief Descriptor Pool Builder.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <optional>
#include <vector>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DescriptorPool;

class DescriptorPoolBuilder {
public:
    struct NoImplicitFreeDescriptorFlag {};
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
private:
    VkDevice m_device;
public:
    DescriptorPoolBuilder(VkDevice logical_device);

    void addDescriptorPoolSize(VkDescriptorType type, uint32_t size);

    DescriptorPool create(uint32_t max_sets, VkDescriptorPoolCreateFlags flags = 0);

    DescriptorPool create(uint32_t max_sets, VkDescriptorPoolCreateFlags flags, NoImplicitFreeDescriptorFlag const&);
};
}
#endif
