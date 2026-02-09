#include <gbVk/Image.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/CommandBuffer.hpp>
#include <gbVk/DebugUtilsObjectName.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Exceptions.hpp>
#include <gbVk/ImageView.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Image::Image(VkDevice logical_device, VkImage image, VkExtent3D const& extent, VkFormat format)
    :m_image(image), m_device(logical_device), m_extent(extent), m_format(format), m_currentAccessMask(0),
     m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED), m_currentQueue(VK_QUEUE_FAMILY_IGNORED), m_hasOwnership(true)
{
}

Image::Image(VkDevice logical_device, VkImage image, VkExtent3D const& extent, VkFormat format, NoOwnership)
    :Image(logical_device, image, extent, format)
{
    m_hasOwnership = false;
}

Image::~Image()
{
    if(m_hasOwnership && m_image) { vkDestroyImage(m_device, m_image, nullptr); }
}

Image::Image(Image&& rhs)
    :m_image(rhs.m_image), m_device(rhs.m_device), m_extent(rhs.m_extent), m_format(rhs.m_format),
     m_currentAccessMask(rhs.m_currentAccessMask), m_currentLayout(rhs.m_currentLayout),
     m_currentQueue(rhs.m_currentQueue), m_hasOwnership(rhs.m_hasOwnership)
{
    rhs.m_image = nullptr;
    rhs.m_device = nullptr;
    rhs.m_extent = { 0, 0, 0 };
    rhs.m_format = VK_FORMAT_UNDEFINED;
    rhs.m_currentAccessMask = 0;
    rhs.m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    rhs.m_currentQueue = VK_QUEUE_FAMILY_IGNORED;
    rhs.m_hasOwnership = false;
}

void Image::setDebugName(char const* name)
{
    DebugUtils::setObjectName(m_device, name, m_image, VK_OBJECT_TYPE_IMAGE);
}

VkImage Image::getVkImage()
{
    return m_image;
}

VkMemoryRequirements Image::getMemoryRequirements()
{
    VkMemoryRequirements ret;
    vkGetImageMemoryRequirements(m_device, m_image, &ret);
    return ret;
}

void Image::transitionLayout(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                             VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask, VkImageLayout old_layout,
                             VkImageLayout new_layout)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Recording);
    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    VkImageMemoryBarrier image_barr;
    image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barr.pNext = nullptr;
    image_barr.srcAccessMask = src_access_mask;
    image_barr.dstAccessMask = dst_access_mask;
    image_barr.oldLayout = old_layout;
    image_barr.newLayout = new_layout;
    image_barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barr.image = m_image;
    image_barr.subresourceRange = range;
    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(), src_stage, dst_stage, 0,
                         0, nullptr, 0, nullptr, 1, &image_barr);
}

void Image::transitionRelease(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                              VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
                              uint32_t dst_queue_family, VkImageLayout old_layout, VkImageLayout new_layout)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Recording);
    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    VkImageMemoryBarrier image_barr;
    image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barr.pNext = nullptr;
    image_barr.srcAccessMask = src_access_mask;
    image_barr.dstAccessMask = 0;
    image_barr.oldLayout = old_layout;
    image_barr.newLayout = new_layout;
    image_barr.srcQueueFamilyIndex = command_buffer.getQueueFamilyIndex();
    image_barr.dstQueueFamilyIndex = dst_queue_family;
    image_barr.image = m_image;
    image_barr.subresourceRange = range;
    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(), src_stage, dst_stage, 0,
                         0, nullptr, 0, nullptr, 1, &image_barr);
}

void Image::transitionAcquire(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                              VkPipelineStageFlags dst_stage, VkAccessFlags dst_access_mask,
                              uint32_t src_queue_family, VkImageLayout old_layout, VkImageLayout new_layout)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Recording);
    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    VkImageMemoryBarrier image_barr;
    image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barr.pNext = nullptr;
    image_barr.srcAccessMask = 0;
    image_barr.dstAccessMask = dst_access_mask;
    image_barr.oldLayout = old_layout;
    image_barr.newLayout = new_layout;
    image_barr.srcQueueFamilyIndex = src_queue_family;
    image_barr.dstQueueFamilyIndex = command_buffer.getQueueFamilyIndex();
    image_barr.image = m_image;
    image_barr.subresourceRange = range;
    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(), src_stage, dst_stage, 0,
                         0, nullptr, 0, nullptr, 1, &image_barr);
}

void Image::transition(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                       VkAccessFlags access_mask, VkImageLayout layout)
{
    VkImageSubresourceRange range;
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;
    transition(command_buffer, src_stage, dst_stage, access_mask, layout, VK_QUEUE_FAMILY_IGNORED, range);
}

void Image::transition(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage,
                       VkAccessFlags access_mask, VkImageLayout layout,
                       uint32_t queue_family, VkImageSubresourceRange const& subresource_range)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Recording);
    VkImageMemoryBarrier image_barr;
    image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barr.pNext = nullptr;
    image_barr.srcAccessMask = m_currentAccessMask;
    image_barr.dstAccessMask = access_mask;
    image_barr.oldLayout = m_currentLayout;
    image_barr.newLayout = layout;
    image_barr.srcQueueFamilyIndex = m_currentQueue;
    image_barr.dstQueueFamilyIndex = queue_family;
    image_barr.image = m_image;
    image_barr.subresourceRange = subresource_range;
    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(), src_stage, dst_stage, 0,
                         0, nullptr, 0, nullptr, 1, &image_barr);

    m_currentAccessMask = access_mask;
    m_currentLayout = layout;
    m_currentQueue = queue_family;
}

uint32_t Image::getWidth() const
{
    return m_extent.width;
}

uint32_t Image::getHeight() const
{
    return m_extent.height;
}

uint32_t Image::getDepth() const
{
    return m_extent.depth;
}

VkExtent3D Image::getExtent() const
{
    return m_extent;
}

VkFormat Image::getFormat() const
{
    return m_format;
}

VkAccessFlags Image::getCurrentAccessMask() const
{
    return m_currentAccessMask;
}

ImageView Image::createImageView()
{
    return createImageView2D(VK_IMAGE_ASPECT_COLOR_BIT);
}

ImageView Image::createImageViewDepthBuffer()
{
    return createImageView2D(VK_IMAGE_ASPECT_DEPTH_BIT);
}

ImageView Image::createImageView2D(VkImageAspectFlags aspect_flags)
{
    return createImageView(VK_IMAGE_VIEW_TYPE_2D, aspect_flags);
}

ImageView Image::createImageView(VkImageViewType view_type, VkImageAspectFlags aspect_flags)
{
    VkImageViewCreateInfo image_view_ci;
    image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_ci.pNext = nullptr;
    image_view_ci.flags = 0;
    image_view_ci.image = m_image;
    image_view_ci.viewType = view_type;
    image_view_ci.format = m_format;
    image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_ci.subresourceRange.aspectMask = aspect_flags;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = 1;
    VkImageView image_view;
    VkResult res = vkCreateImageView(m_device, &image_view_ci, nullptr, &image_view);
    checkVulkanError(res, "Error in vkCreateImageView.");
    return ImageView(m_device, image_view);
}

void Image::copy(CommandBuffer& command_buffer, Buffer& source_buffer, Image& destination_image)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Recording);

    VkBufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = destination_image.m_extent.width;
    region.imageExtent.height = destination_image.m_extent.height;
    region.imageExtent.depth = destination_image.m_extent.depth;

    vkCmdCopyBufferToImage(command_buffer.getVkCommandBuffer(), source_buffer.getVkBuffer(),
        destination_image.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void Image::copy(CommandBuffer& command_buffer, Image& source_image, Image& destination_image)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Recording);
    GHULBUS_PRECONDITION(source_image.m_format == destination_image.m_format);
    GHULBUS_PRECONDITION(source_image.m_extent.width == destination_image.m_extent.width);
    GHULBUS_PRECONDITION(source_image.m_extent.height == destination_image.m_extent.height);
    GHULBUS_PRECONDITION(source_image.m_extent.depth == destination_image.m_extent.depth);

    VkImageCopy region;
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffset.x = 0;
    region.srcOffset.y = 0;
    region.srcOffset.z = 0;
    region.dstSubresource = region.srcSubresource;
    region.dstOffset = region.srcOffset;
    region.extent.width = source_image.m_extent.width;
    region.extent.height = source_image.m_extent.height;
    region.extent.depth = source_image.m_extent.depth;
    vkCmdCopyImage(command_buffer.getVkCommandBuffer(),
                   source_image.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   destination_image.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &region);
}

void Image::blit(CommandBuffer& command_buffer, Image& source_image, Image& destination_image)
{
    GHULBUS_PRECONDITION(command_buffer.getCurrentState() == CommandBuffer::State::Recording);

    VkImageBlit region;
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.layerCount = 1;
    region.srcOffsets[0].x = 0;
    region.srcOffsets[0].y = 0;
    region.srcOffsets[0].z = 0;
    region.srcOffsets[1].x = source_image.m_extent.width;
    region.srcOffsets[1].y = source_image.m_extent.height;
    region.srcOffsets[1].z = source_image.m_extent.depth;
    region.dstSubresource = region.srcSubresource;
    region.dstOffsets[0] = region.srcOffsets[0];
    region.dstOffsets[1].x = destination_image.m_extent.width;
    region.dstOffsets[1].y = destination_image.m_extent.height;
    region.dstOffsets[1].z = destination_image.m_extent.depth;
    vkCmdBlitImage(command_buffer.getVkCommandBuffer(),
                   source_image.m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   destination_image.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1, &region, VK_FILTER_NEAREST);
}
}
