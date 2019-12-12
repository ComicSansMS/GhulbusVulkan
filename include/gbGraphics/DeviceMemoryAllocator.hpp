#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DEVICE_MEMORY_ALLOCATOR_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DEVICE_MEMORY_ALLOCATOR_HPP

/** @file
*
* @brief Device Memory Allocator.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <memory>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
enum class MemoryUsage {
    GpuOnly,
    CpuOnly,
    CpuToGpu,
    GpuToCpu
};

class DeviceMemoryAllocator {
protected:
    struct HandleConcept {
        virtual ~HandleConcept() = 0;
        virtual VkDeviceMemory getVkDeviceMemory() const = 0;
        virtual VkDeviceSize getOffset() const = 0;
        virtual VkDeviceSize getSize() const = 0;
    };
public:
    class Handle {
    private:
        std::unique_ptr<HandleConcept> m_handle;
    public:
        Handle() = default;

        Handle(std::unique_ptr<HandleConcept>&& handle);

        ~Handle();

        Handle(Handle const&) = delete;
        Handle& operator=(Handle const&) = delete;

        Handle(Handle&&) = default;
        Handle& operator=(Handle&&) = default;

        VkDeviceMemory getVkDeviceMemory() const;
        VkDeviceSize getOffset() const;
        VkDeviceSize getSize() const;
    };
protected:
    DeviceMemoryAllocator() = default;
public:
    virtual ~DeviceMemoryAllocator() = default;

    DeviceMemoryAllocator(DeviceMemoryAllocator const&) = delete;
    DeviceMemoryAllocator& operator=(DeviceMemoryAllocator const&) = delete;

    DeviceMemoryAllocator(DeviceMemoryAllocator&& rhs) = delete;
    DeviceMemoryAllocator& operator=(DeviceMemoryAllocator&& rhs) = delete;

    virtual Handle allocateMemoryForImage(GhulbusVulkan::Image& image, MemoryUsage usage) = 0;
    virtual Handle allocateMemoryForImage(GhulbusVulkan::Image& image,
                                          VkMemoryRequirements const& requirements,
                                          VkMemoryPropertyFlags required_flags) = 0;
};
}
#endif
