#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_IMAGE_2D_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_IMAGE_2D_HPP

/** @file
*
* @brief Image 2D.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>
#include <gbGraphics/GenericImage.hpp>

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
    GhulbusGraphics::GenericImage m_genImage;
public:
    Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height);

    Image2d(GraphicsInstance& instance, uint32_t width, uint32_t height, VkImageTiling tiling,
            VkImageUsageFlags image_usage, MemoryUsage memory_usage);

    ~Image2d() = default;

    Image2d(Image2d&&) = default;

    uint32_t getWidth() const;
    uint32_t getHeight() const;

    bool isMappable() const;

    GhulbusVulkan::MappedMemory map();

    GhulbusVulkan::MappedMemory map(VkDeviceSize offset, VkDeviceSize size);

    GhulbusVulkan::ImageView createImageView();

    GhulbusVulkan::SubmitStaging setDataAsynchronously(std::byte const* data,
                                                       std::optional<uint32_t> target_queue = std::nullopt);

    GhulbusVulkan::Image& getImage();
};
}
#endif

