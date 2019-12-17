#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MEMORY_BUFFER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MEMORY_BUFFER_HPP

/** @file
*
* @brief Memory Buffer.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/MappedMemory.hpp>
#include <gbVk/MemoryUsage.hpp>
#include <gbVk/SubmitStaging.hpp>

#include <cstdint>
#include <optional>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
using GhulbusVulkan::MemoryUsage;

class GraphicsInstance;

class MemoryBuffer {
private:
    GhulbusVulkan::Buffer m_buffer;
    GhulbusVulkan::DeviceMemory m_deviceMemory;
    GhulbusGraphics::GraphicsInstance* m_instance;
    VkDeviceSize m_size;
    VkBufferUsageFlags m_bufferUsage;
    MemoryUsage m_memoryUsage;
public:
    MemoryBuffer(GraphicsInstance& instance, VkDeviceSize size,
                 VkBufferUsageFlags buffer_usage, MemoryUsage memory_usage);

    MemoryBuffer(GraphicsInstance& instance, VkDeviceSize size,
                 VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags required_flags);

    ~MemoryBuffer();

    MemoryBuffer(MemoryBuffer&&) = default;

    bool isMappable() const;

    GhulbusVulkan::MappedMemory map();

    GhulbusVulkan::MappedMemory map(VkDeviceSize offset, VkDeviceSize size);

    GhulbusVulkan::SubmitStaging setDataAsynchronously(std::byte const* data,
                                                       std::optional<uint32_t> target_queue = std::nullopt);

    VkDeviceSize getSize() const;

    GhulbusVulkan::Buffer& getBuffer();
};
}
#endif

