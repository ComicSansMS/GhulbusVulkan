#include <gbVk/Sampler.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

Sampler::Sampler(VkDevice logical_device, VkSampler sampler)
    :m_sampler(sampler), m_device(logical_device)
{
}

Sampler::Sampler(Sampler&& rhs)
    :m_sampler(rhs.m_sampler), m_device(rhs.m_device)
{
    rhs.m_sampler = nullptr;
}

Sampler::~Sampler()
{
    if (m_sampler) {
        vkDestroySampler(m_device, m_sampler, nullptr);
    }
}

VkSampler Sampler::getVkSampler()
{
    return m_sampler;
}

}


