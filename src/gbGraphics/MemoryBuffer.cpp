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
    m_deviceMemory.bindBuffer(m_buffer);
}

MemoryBuffer::MemoryBuffer(GraphicsInstance& instance, VkDeviceSize size,
                           VkBufferUsageFlags buffer_usage, VkMemoryPropertyFlags required_flags)
    :m_buffer(instance.getVulkanDevice().createBuffer(size, buffer_usage)),
     m_deviceMemory(instance.getDeviceMemoryAllocator().allocateMemoryForBuffer(m_buffer, required_flags)),
     m_instance(&instance), m_size(size), m_bufferUsage(buffer_usage), m_memoryUsage(MemoryUsage::CpuOnly)
{
    m_deviceMemory.bindBuffer(m_buffer);
}

MemoryBuffer::~MemoryBuffer()
{
    // make sure buffer is destroyed first, before the memory that backs it up
    GhulbusVulkan::Buffer destroyer(std::move(m_buffer));
}

bool MemoryBuffer::isMappable() const
{
    return (m_memoryUsage == MemoryUsage::CpuOnly) ||
           (m_memoryUsage == MemoryUsage::CpuToGpu) ||
           (m_memoryUsage == MemoryUsage::GpuToCpu);
}

VkBufferUsageFlags MemoryBuffer::getBufferUsage() const
{
    return m_bufferUsage;
}

MemoryUsage MemoryBuffer::getMemoryUsage() const
{
    return m_memoryUsage;
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

GhulbusVulkan::SubmitStaging MemoryBuffer::setDataAsynchronously(std::byte const* data,
                                                                 std::optional<uint32_t> target_queue)
{
    GhulbusGraphics::MemoryBuffer staging_buffer(*m_instance, m_size,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryUsage::CpuOnly);
    {
        auto mapped_mem = staging_buffer.map();
        std::memcpy(mapped_mem, data, m_size);
    }
    auto command_buffers = m_instance->getCommandPoolRegistry().allocateCommandBuffersTransfer_Transient(1);
    auto& command_buffer = command_buffers.getCommandBuffer(0);

    command_buffer.begin();
    VkBufferCopy buffer_copy;
    buffer_copy.srcOffset = 0;
    buffer_copy.dstOffset = 0;
    buffer_copy.size = m_size;
    vkCmdCopyBuffer(command_buffer.getVkCommandBuffer(), staging_buffer.getBuffer().getVkBuffer(),
                    m_buffer.getVkBuffer(), 1, &buffer_copy);
    if (target_queue) {
        m_buffer.transitionRelease(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, *target_queue);
    }
    command_buffer.end();

    GhulbusVulkan::SubmitStaging ret;
    ret.addCommandBuffers(command_buffers);
    ret.adoptResources(std::move(command_buffers), std::move(staging_buffer));
    return ret;
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
