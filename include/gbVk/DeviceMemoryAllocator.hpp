#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_ALLOCATOR_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_ALLOCATOR_HPP

/** @file
*
* @brief Device Memory Allocator Interface.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <memory>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Image;

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
        virtual void* mapMemory(VkDeviceSize offset, VkDeviceSize size) = 0;
        virtual void unmapMemory(void* mapped_memory) = 0;
        virtual void flush(VkDeviceSize offset, VkDeviceSize size) = 0;
        virtual void invalidate(VkDeviceSize offset, VkDeviceSize size) = 0;
    };
public:
    class DeviceMemory;
protected:
    DeviceMemoryAllocator() = default;
public:
    virtual ~DeviceMemoryAllocator() = default;

    DeviceMemoryAllocator(DeviceMemoryAllocator const&) = delete;
    DeviceMemoryAllocator& operator=(DeviceMemoryAllocator const&) = delete;

    DeviceMemoryAllocator(DeviceMemoryAllocator&& rhs) = delete;
    DeviceMemoryAllocator& operator=(DeviceMemoryAllocator&& rhs) = delete;

    virtual DeviceMemory allocateMemory(size_t requested_size, VkMemoryPropertyFlags flags) = 0;
    virtual DeviceMemory allocateMemory(VkMemoryRequirements const& requirements,
                                        VkMemoryPropertyFlags required_flags) = 0;

    virtual DeviceMemory allocateMemoryForImage(Image& image, MemoryUsage usage) = 0;
    virtual DeviceMemory allocateMemoryForImage(Image& image,
                                                VkMemoryRequirements const& requirements,
                                                VkMemoryPropertyFlags required_flags) = 0;
};

class DeviceMemoryAllocator::DeviceMemory {
public:
    class MappedMemory;
private:
    using HandleConcept = DeviceMemoryAllocator::HandleConcept;
    std::unique_ptr<HandleConcept> m_handle;
public:
    DeviceMemory() = default;

    DeviceMemory(std::unique_ptr<HandleConcept>&& handle);

    ~DeviceMemory();

    DeviceMemory(DeviceMemory const&) = delete;
    DeviceMemory& operator=(DeviceMemory const&) = delete;

    DeviceMemory(DeviceMemory&&) = default;
    DeviceMemory& operator=(DeviceMemory&&) = default;

    VkDeviceMemory getVkDeviceMemory() const;
    VkDeviceSize getOffset() const;
    VkDeviceSize getSize() const;

    MappedMemory map();
    MappedMemory map(VkDeviceSize offset, VkDeviceSize size);
};

class DeviceMemoryAllocator::DeviceMemory::MappedMemory {
private:
    void* m_mappedMemory;
    HandleConcept* m_handle;
public:
    MappedMemory(HandleConcept& handle, void* mapped_memory);

    ~MappedMemory();

    MappedMemory(MappedMemory&& rhs);
    MappedMemory& operator=(MappedMemory&& rhs);

    MappedMemory(MappedMemory const&) = delete;
    MappedMemory& operator=(MappedMemory const&) = delete;

    operator std::byte*();
    std::byte& operator[](std::size_t index);

    operator std::byte const*() const;
    std::byte operator[](std::size_t index) const;

    void flush();
    void invalidate();
};

}
#endif
