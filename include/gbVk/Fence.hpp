#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_FENCE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_FENCE_HPP

/** @file
*
* @brief Fence.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <chrono>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Fence {
public:
    enum Status {
        Ready,
        NotReady
    };
private:
    VkFence m_fence;
    VkDevice m_device;
public:
    Fence(VkDevice logical_device, VkFence fence);

    ~Fence();

    Fence(Fence const&) = delete;
    Fence& operator=(Fence const&) = delete;

    Fence(Fence&& rhs);
    Fence& operator=(Fence&&) = delete;

    VkFence getVkFence();

    Status getStatus();

    void wait();

    Status wait_for(std::chrono::nanoseconds timeout);

    void reset();
};
}
#endif
