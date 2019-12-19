#include <gbGraphics/Image2d.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/GraphicsInstance.hpp>
#include <gbGraphics/MemoryBuffer.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/ImageView.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Image2d::Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height)
    : Image2d(instance, width, height, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, MemoryUsage::GpuOnly)
{}

Image2d::Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height, VkImageTiling tiling,
                 VkImageUsageFlags image_usage, MemoryUsage memory_usage)
    : m_genImage(instance, VkExtent3D{ width, height, 1 }, VK_FORMAT_R8G8B8A8_UNORM, 1, 1, VK_SAMPLE_COUNT_1_BIT,
                 tiling, image_usage, memory_usage)
{
}

GhulbusVulkan::Image& Image2d::getImage()
{
    return m_genImage.getImage();
}

uint32_t Image2d::getWidth() const
{
    return m_genImage.getExtent().width;
}

uint32_t Image2d::getHeight() const
{
    return m_genImage.getExtent().height;
}

bool Image2d::isMappable() const
{
    return m_genImage.isMappable();
}

GhulbusVulkan::MappedMemory Image2d::map()
{
    return m_genImage.map();
}

GhulbusVulkan::MappedMemory Image2d::map(VkDeviceSize offset, VkDeviceSize size)
{
    return m_genImage.map(offset, size);
}

GhulbusVulkan::ImageView Image2d::createImageView()
{
    return m_genImage.createImageView(VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
}

GhulbusVulkan::SubmitStaging Image2d::setDataAsynchronously(std::byte const* data,
                                                            std::optional<uint32_t> target_queue)
{
    GHULBUS_ASSERT(m_genImage.getFormat() == VK_FORMAT_R8G8B8A8_UNORM);
    VkExtent3D const image_extent = m_genImage.getExtent();
    auto const texture_width = image_extent.width;
    auto const texture_height = image_extent.height;
    VkDeviceSize const texture_size = texture_width * texture_height * 4;

    GhulbusGraphics::GraphicsInstance& instance = m_genImage.getInstance();
    GhulbusGraphics::MemoryBuffer staging_buffer(instance, texture_size,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryUsage::CpuOnly);
    {
        auto mapped_mem = staging_buffer.map();
        std::memcpy(mapped_mem, data, texture_size);
    }
    auto command_buffers = instance.getCommandPoolRegistry().allocateCommandBuffersTransfer_Transient(1);
    auto command_buffer = command_buffers.getCommandBuffer(0);

    GhulbusVulkan::Image& image = m_genImage.getImage();

    command_buffer.begin();
    image.transitionLayout(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, VK_ACCESS_TRANSFER_WRITE_BIT,
                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    GhulbusVulkan::Image::copy(command_buffer, staging_buffer.getBuffer(), image);

    if (!target_queue) {
        image.transitionLayout(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    } else {
        image.transitionRelease(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                  VK_ACCESS_TRANSFER_WRITE_BIT, *target_queue,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
    command_buffer.end();

    GhulbusVulkan::SubmitStaging ret;
    ret.addCommandBuffers(command_buffers);
    ret.adoptResources(std::move(command_buffers), std::move(staging_buffer));
    return ret;
}
}
