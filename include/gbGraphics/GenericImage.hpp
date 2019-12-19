#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GENERIC_IMAGE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GENERIC_IMAGE_HPP

/** @file
*
* @brief Generic Image.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/DeviceMemory.hpp>
#include <gbVk/ForwardDecl.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/MappedMemory.hpp>
#include <gbVk/MemoryUsage.hpp>

#include <cstdint>
#include <optional>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
using GhulbusVulkan::MemoryUsage;

class GraphicsInstance;

class GenericImage {
public:
    struct NoDeviceMemory_T {};
    static constexpr NoDeviceMemory_T noDeviceMemory = {};
private:
    GhulbusVulkan::Image m_image;
    std::optional<GhulbusVulkan::DeviceMemory> m_deviceMemory;
    GhulbusGraphics::GraphicsInstance* m_instance;
    uint32_t m_mipLevels;
    uint32_t m_arrayLayers;
    VkSampleCountFlagBits m_samples;
    VkImageTiling m_tiling;
    VkImageUsageFlags m_imageUsage;
    MemoryUsage m_memoryUsage;
public:
    GenericImage(GraphicsInstance& instance, VkExtent3D dimensions, VkFormat format, uint32_t mip_levels,
                 uint32_t array_layers, VkSampleCountFlagBits samples, VkImageTiling tiling,
                 VkImageUsageFlags image_usage, MemoryUsage memory_usage);

    // @todo: non-owning mode
    //GenericImage(GraphicsInstance& instance, GhulbusVulkan::Image image, VkImageTiling tiling, NoDeviceMemory_T);

    ~GenericImage();

    GenericImage(GenericImage&&) = default;

    VkFormat getFormat() const;
    VkExtent3D getExtent() const;
    uint32_t getMipLevels() const;
    uint32_t getArrayLayers() const;
    VkSampleCountFlagBits getSamples() const;
    VkImageTiling getTiling() const;
    VkImageUsageFlags getUsage() const;

    bool isMappable() const;

    GhulbusVulkan::MappedMemory map();

    GhulbusVulkan::MappedMemory map(VkDeviceSize offset, VkDeviceSize size);

    GhulbusVulkan::ImageView createImageView(VkImageViewType view_type, VkImageAspectFlags aspect_flags);

    GhulbusGraphics::GraphicsInstance& getInstance();

    GhulbusVulkan::Image& getImage();
};
}
#endif

