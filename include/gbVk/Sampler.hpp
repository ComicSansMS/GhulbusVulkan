#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SAMPLER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SAMPLER_HPP

/** @file
*
* @brief Sampler.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

class Sampler {
private:
    VkSampler m_sampler;
    VkDevice m_device;
public:
    explicit Sampler(VkDevice logical_device, VkSampler sampler);

    ~Sampler();

    Sampler(Sampler const&) = delete;
    Sampler& operator=(Sampler const&) = delete;

    Sampler(Sampler&& rhs);
    Sampler& operator=(Sampler&&) = delete;

    VkSampler getVkSampler();

};
}
#endif
