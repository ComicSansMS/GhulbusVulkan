#include <gbGraphics/Image2d.hpp>

#include <gbGraphics/GraphicsInstance.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/DeviceMemoryAllocator.hpp>
#include <gbVk/Image.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

BaseImage::BaseImage(GraphicsInstance& instance)
{
    GhulbusVulkan::Device& device = instance.getVulkanDevice();
    GhulbusVulkan::Image image = device.createImage2D(128, 128);
    auto const memory = instance.getDeviceMemoryAllocator().allocateMemoryForImage(image, GhulbusVulkan::MemoryUsage::CpuToGpu);
}

}
