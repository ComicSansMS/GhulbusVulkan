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

namespace GHULBUS_VULKAN_NAMESPACE
{

class Image {
private:
    VkImage m_image;
    VkDevice m_device;
public:
    Image(VkDevice logical_device, VkImage image);
    ~Image();

    Image(Image const&) = delete;
    Image& operator=(Image const&) = delete;

    Image(Image&& rhs);
    Image& operator=(Image&& rhs) = delete;

    VkImage getVkImage();

    VkMemoryRequirements getMemoryRequirements();
};
}
#endif
