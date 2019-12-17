#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_IMAGE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_IMAGE_HPP

/** @file
*
* @brief Image.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <gbVk/DeviceMemory.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Buffer;
class CommandBuffer;
class ImageView;

class Image {
public:
    struct NoOwnership {};
private:
    VkImage m_image;
    VkDevice m_device;
    VkExtent3D m_extent;
    VkFormat m_format;
    VkAccessFlags m_currentAccessMask;
    VkImageLayout m_currentLayout;
    uint32_t m_currentQueue;
    bool m_hasOwnership;
public:
    Image(VkDevice logical_device, VkImage image, VkExtent3D const& extent, VkFormat format);

    Image(VkDevice logical_device, VkImage image, VkExtent3D const& extent, VkFormat format, NoOwnership);
    ~Image();

    Image(Image const&) = delete;
    Image& operator=(Image const&) = delete;

    Image(Image&& rhs);
    Image& operator=(Image&& rhs) = delete;

    VkImage getVkImage();

    VkMemoryRequirements getMemoryRequirements();

    void transition(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                    VkAccessFlags access_mask, VkImageLayout layout);
    void transition(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                    VkAccessFlags access_mask, VkImageLayout layout,
                    uint32_t queue_family, VkImageSubresourceRange const& subresource_range);

    uint32_t getWidth() const;
    uint32_t getHeight() const;
    uint32_t getDepth() const;
    VkFormat getFormat() const;
    VkAccessFlags getCurrentAccessMask() const;

    ImageView createImageView();
    ImageView createImageViewDepthBuffer();
    ImageView createImageView2D(VkImageAspectFlags aspect_flags);
    ImageView createImageView(VkImageViewType view_type, VkImageAspectFlags aspect_flags);

    static void copy(CommandBuffer& command_buffer, Buffer& source_buffer, Image& destination_image);
    static void copy(CommandBuffer& command_buffer, Image& source_image, Image& destination_image);
    static void blit(CommandBuffer& command_buffer, Image& source_image, Image& destination_image);
};
}
#endif
