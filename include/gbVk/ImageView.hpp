#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_IMAGE_VIEW_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_IMAGE_VIEW_HPP

/** @file
*
* @brief Image view.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class ImageView {
private:
    VkImageView m_image_view;
    VkDevice m_device;
public:
    ImageView(VkDevice logical_device, VkImageView image_view);

    ~ImageView();

    ImageView(ImageView const&) = delete;
    ImageView& operator=(ImageView const&) = delete;

    ImageView(ImageView&& rhs);
    ImageView& operator=(ImageView&& rhs) = delete;

    VkImageView getVkImageView();
};
}
#endif
