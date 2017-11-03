#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_DEVICE_MEMORY_HPP

/** @file
*
* @brief Device Memory.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <cstddef>

namespace GHULBUS_VULKAN_NAMESPACE
{
class DeviceMemory {
public:
    class MappedMemory {
    private:
        void* m_mappedMemory;
        VkDevice m_device;
        VkDeviceMemory m_memory;
    public:
        MappedMemory(VkDevice device, VkDeviceMemory memory, void* mapped_memory);

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
private:
    VkDeviceMemory m_memory;
    VkDevice m_device;
public:
    DeviceMemory(VkDevice device, VkDeviceMemory memory);

    ~DeviceMemory();

    DeviceMemory(DeviceMemory const&) = delete;
    DeviceMemory& operator=(DeviceMemory const&) = delete;

    DeviceMemory(DeviceMemory&&);
    DeviceMemory& operator=(DeviceMemory&&) = delete;

    VkDeviceMemory getVkDeviceMemory();

    VkDeviceSize getCommitment();

    MappedMemory map();
    MappedMemory map(VkDeviceSize offset, VkDeviceSize size);
};
}
#endif
