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
#include <concepts>
#include <cstddef>
#include <span>

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

    void setDebugName(char const* name);

    VkFence getVkFence();

    Status getStatus();

    void wait();

    Status wait_for(std::chrono::nanoseconds timeout);

    void reset();

    template<typename... Args> requires(std::same_as<Args, Fence> &&...)
    static Status wait_all(Args&... fences) {
        return wait_all_for(std::chrono::nanoseconds::max(), fences...);
    }

    template<typename... Args> requires(std::same_as<Args, Fence> &&...)
    static Status wait_all_for(std::chrono::nanoseconds timeout, Args&... fences) {
        static_assert(sizeof...(Args) < 1024);
        VkFence vk_fences[sizeof...(Args)] = { fences.getVkFence()... };
        VkDevice vk_devices[sizeof...(Args)] = { fences.m_device... };
        return (wait_impl(vk_devices, vk_fences, timeout, true) == 0) ? Status::Ready : Status::NotReady;
    }

    template<typename... Args> requires(std::same_as<Args, Fence> &&...)
    static std::size_t wait_any(Args&... fences) {
        return wait_any_for(std::chrono::nanoseconds::max(), fences...);
    }

    template<typename... Args> requires(std::same_as<Args, Fence> &&...)
    static std::size_t wait_any_for(std::chrono::nanoseconds timeout, Args&... fences) {
        static_assert(sizeof...(Args) < 1024);
        VkFence vk_fences[sizeof...(Args)] = { fences.getVkFence()... };
        VkDevice vk_devices[sizeof...(Args)] = { fences.m_device... };
        return wait_impl(vk_devices, vk_fences, timeout, false);
    }

private:
    static size_t wait_impl(std::span<VkDevice> vk_devices, std::span<VkFence> vk_fences,
                            std::chrono::nanoseconds timeout, bool wait_all);
};
}
#endif
