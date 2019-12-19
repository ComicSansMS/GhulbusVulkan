#include <gbGraphics/GenericImage.hpp>

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

GenericImage::GenericImage(GraphicsInstance& instance, VkExtent3D dimensions, VkFormat format, uint32_t mip_levels,
                           uint32_t array_layers, VkSampleCountFlagBits samples, VkImageTiling tiling,
                           VkImageUsageFlags image_usage, MemoryUsage memory_usage)
    :m_image(instance.getVulkanDevice().createImage(dimensions, format, mip_levels, array_layers,
                                                    samples, tiling, image_usage)),
    m_deviceMemory(instance.getDeviceMemoryAllocator().allocateMemoryForImage(m_image, memory_usage)),
    m_instance(&instance), m_mipLevels(mip_levels), m_arrayLayers(array_layers), m_samples(samples),
    m_tiling(tiling), m_imageUsage(image_usage), m_memoryUsage(memory_usage)
{
    m_deviceMemory->bindImage(m_image);
}

GenericImage::~GenericImage()
{
    // make sure image is destroyed first, before the memory that backs it up
    GhulbusVulkan::Image destroyer(std::move(m_image));
}

VkFormat GenericImage::getFormat() const
{
    return m_image.getFormat();
}

VkExtent3D GenericImage::getExtent() const
{
    return m_image.getExtent();
}

uint32_t GenericImage::getMipLevels() const
{
    return m_mipLevels;
}

uint32_t GenericImage::getArrayLayers() const
{
    return m_arrayLayers;
}

VkSampleCountFlagBits GenericImage::getSamples() const
{
    return m_samples;
}

VkImageTiling GenericImage::getTiling() const
{
    return m_tiling;
}

VkImageUsageFlags GenericImage::getUsage() const
{
    return m_imageUsage;
}

bool GenericImage::isMappable() const
{
    return m_deviceMemory && ((m_memoryUsage == MemoryUsage::CpuOnly) ||
                              (m_memoryUsage == MemoryUsage::CpuToGpu) ||
                              (m_memoryUsage == MemoryUsage::GpuToCpu));
}

GhulbusVulkan::MappedMemory GenericImage::map()
{
    GHULBUS_PRECONDITION(isMappable());
    return m_deviceMemory->map();
}

GhulbusVulkan::MappedMemory GenericImage::map(VkDeviceSize offset, VkDeviceSize size)
{
    GHULBUS_PRECONDITION(isMappable());
    return m_deviceMemory->map(offset, size);
}

GhulbusVulkan::ImageView GenericImage::createImageView(VkImageViewType view_type, VkImageAspectFlags aspect_flags)
{
    return m_image.createImageView(view_type, aspect_flags);
}

GhulbusGraphics::GraphicsInstance& GenericImage::getInstance()
{
    return *m_instance;
}

GhulbusVulkan::Image& GenericImage::getImage()
{
    return m_image;
}
}
