#include <gbGraphics/Image2d.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/GraphicsInstance.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Device.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Image2d::Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height)
    :Image2d(instance, width, height, VK_IMAGE_TILING_OPTIMAL,
             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, MemoryUsage::GpuOnly)
{}

Image2d::Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height, VkImageTiling tiling,
    VkImageUsageFlags image_usage, MemoryUsage memory_usage)
    :m_image(instance.getVulkanDevice().createImage(VkExtent3D{ width, height, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
        1, 1, tiling, image_usage)),
    m_deviceMemory(instance.getDeviceMemoryAllocator().allocateMemoryForImage(m_image, memory_usage)),
    m_instance(&instance), m_tiling(tiling), m_imageUsage(image_usage), m_memoryUsage(memory_usage)
{
    m_image.bindMemory(*m_deviceMemory);
}

Image2d::~Image2d()
{
    // make sure image is destroyed first, before the memory that backs it up
    GhulbusVulkan::Image destroyer(std::move(m_image));
}

GhulbusVulkan::Image& Image2d::getImage()
{
    return m_image;
}

bool Image2d::isMappable() const
{
    return m_deviceMemory && ((m_memoryUsage == MemoryUsage::CpuOnly) ||
                              (m_memoryUsage == MemoryUsage::CpuToGpu) ||
                              (m_memoryUsage == MemoryUsage::GpuToCpu));
}

GhulbusVulkan::MappedMemory Image2d::map()
{
    GHULBUS_PRECONDITION(isMappable());
    return m_deviceMemory->map();
}

GhulbusVulkan::MappedMemory Image2d::map(VkDeviceSize offset, VkDeviceSize size)
{
    GHULBUS_PRECONDITION(isMappable());
    return m_deviceMemory->map(offset, size);
}

GhulbusVulkan::SubmitStaging Image2d::setDataAsynchronously(std::byte const* data)
{
    GHULBUS_ASSERT(m_image.getFormat() == VK_FORMAT_R8G8B8A8_UNORM);
    auto const texture_width = m_image.getWidth();
    auto const texture_height = m_image.getHeight();
    VkDeviceSize const texture_size = texture_width * texture_height * 4;
    GhulbusVulkan::Device& device = m_instance->getVulkanDevice();

    auto texture_staging_buffer = device.createBuffer(texture_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto texture_staging_buffer_mem_reqs = texture_staging_buffer.getMemoryRequirements();
    /// @todo use instance allocator
    /// @todo use buffer object
    auto texture_staging_memory = device.allocateMemory(texture_staging_buffer_mem_reqs,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    texture_staging_buffer.bindBufferMemory(texture_staging_memory);
    {
        auto mapped_mem = texture_staging_memory.map();
        std::memcpy(mapped_mem, data, texture_size);
    }

    auto command_buffers = m_instance->getCommandPoolRegistry().allocateGraphicCommandBuffers_Transient(1);
    auto command_buffer = command_buffers.getCommandBuffer(0);

    command_buffer.begin();
    m_image.transition(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    GhulbusVulkan::Image::copy(command_buffer, texture_staging_buffer, m_image);
    m_image.transition(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    command_buffer.end();

    GhulbusVulkan::SubmitStaging ret;
    ret.addCommandBuffers(command_buffers);
    ret.adoptResources(std::move(command_buffers), std::move(texture_staging_memory),
        std::move(texture_staging_buffer));
    return ret;
}
}
