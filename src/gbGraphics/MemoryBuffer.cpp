#include <gbGraphics/MemoryBuffer.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/GraphicsInstance.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Device.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
MemoryBuffer::MemoryBuffer(GraphicsInstance& instance, VkDeviceSize size,
                           VkBufferUsageFlags buffer_usage, MemoryUsage memory_usage)
    :m_buffer(instance.getVulkanDevice().createBuffer(size, buffer_usage)),
     m_deviceMemory(instance.getDeviceMemoryAllocator().allocateMemoryForBuffer(m_buffer, memory_usage)),
     m_instance(&instance), m_size(size), m_bufferUsage(buffer_usage), m_memoryUsage(memory_usage)
{
    m_buffer.bindBufferMemory(m_deviceMemory, m_deviceMemory.getOffset());
}

MemoryBuffer::MemoryBuffer(GraphicsInstance& instance, VkDeviceSize size,
                           VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags required_flags)
    :m_buffer(instance.getVulkanDevice().createBuffer(size, buffer_usage)),
     m_deviceMemory(instance.getDeviceMemoryAllocator().allocateMemoryForBuffer(m_buffer, required_flags)),
     m_instance(&instance), m_size(size), m_bufferUsage(buffer_usage), m_memoryUsage(MemoryUsage::CpuOnly)
{
    m_buffer.bindBufferMemory(m_deviceMemory, m_deviceMemory.getOffset());
}

MemoryBuffer::~MemoryBuffer()
{
    // make sure buffer is destroyed first, before the memory that backs it up
    GhulbusVulkan::Buffer destroyer(std::move(m_buffer));
}

//GhulbusVulkan::SubmitStaging setDataAsynchronously(std::byte const* data);

bool MemoryBuffer::isMappable() const
{
    return (m_memoryUsage == MemoryUsage::CpuOnly) ||
           (m_memoryUsage == MemoryUsage::CpuToGpu) ||
           (m_memoryUsage == MemoryUsage::GpuToCpu);
}

GhulbusVulkan::MappedMemory MemoryBuffer::map()
{
    GHULBUS_PRECONDITION(isMappable());
    return m_deviceMemory.map();
}

GhulbusVulkan::MappedMemory MemoryBuffer::map(VkDeviceSize offset, VkDeviceSize size)
{
    GHULBUS_PRECONDITION(isMappable());
    return m_deviceMemory.map(offset, size);
}

VkDeviceSize MemoryBuffer::getSize() const
{
    return m_size;
}

GhulbusVulkan::Buffer& MemoryBuffer::getBuffer()
{
    return m_buffer;
}
}
