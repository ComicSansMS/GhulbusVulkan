#include <gbGraphics/Image2d.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/GraphicsInstance.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Device.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

Image2d::Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height, VkImageTiling tiling,
    VkImageUsageFlags image_usage, GhulbusVulkan::MemoryUsage memory_usage)
    :m_image(instance.getVulkanDevice().createImage(VkExtent3D{ width, height, 1 }, VK_FORMAT_R8G8B8A8_UNORM,
        1, 1, tiling, image_usage)),
    m_deviceMemory(instance.getDeviceMemoryAllocator().allocateMemoryForImage(m_image, memory_usage)),
    m_instance(&instance), m_tiling(tiling), m_imageUsage(image_usage), m_memoryUsage(memory_usage)
{
    m_image.bindMemory(*m_deviceMemory);
}

Image2d::~Image2d()
{
    // make sure image is destroy first, before the memory that backs it up
    GhulbusVulkan::Image destroyer(std::move(m_image));
}

GhulbusVulkan::Image& Image2d::getImage()
{
    return m_image;
}

bool Image2d::isMappable() const
{
    using GhulbusVulkan::MemoryUsage;
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
}
