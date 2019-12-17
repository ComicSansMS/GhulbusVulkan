#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_IMAGE_2D_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_IMAGE_2D_HPP

/** @file
*
* @brief Image 2D.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/MappedMemory.hpp>
#include <gbVk/MemoryUsage.hpp>
#include <gbVk/SubmitStaging.hpp>

#include <cstdint>
#include <optional>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
using GhulbusVulkan::MemoryUsage;

class GraphicsInstance;

class Image2d {
public:
    struct NoDeviceMemory_T {};
    static constexpr NoDeviceMemory_T noDeviceMemory = {};
private:
    GhulbusVulkan::Image m_image;
    std::optional<GhulbusVulkan::DeviceMemory> m_deviceMemory;
    GhulbusGraphics::GraphicsInstance* m_instance;
    VkImageTiling m_tiling;
    VkImageUsageFlags m_imageUsage;
    MemoryUsage m_memoryUsage;
public:
    Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height);

    Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height, VkImageTiling tiling,
            VkImageUsageFlags image_usage, MemoryUsage memory_usage);

    Image2d(GraphicsInstance& instance, GhulbusVulkan::Image image, VkImageTiling tiling, NoDeviceMemory_T);

    ~Image2d();

    Image2d(Image2d&&) = default;

    bool isMappable() const;

    GhulbusVulkan::MappedMemory map();

    GhulbusVulkan::MappedMemory map(VkDeviceSize offset, VkDeviceSize size);

    GhulbusVulkan::SubmitStaging setDataAsynchronously(std::byte const* data,
                                                       std::optional<uint32_t> target_queue = std::nullopt);

    GhulbusVulkan::Image& getImage();
};
}
#endif

