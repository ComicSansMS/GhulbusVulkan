#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_FRAMEBUFFER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_FRAMEBUFFER_HPP

/** @file
*
* @brief Framebuffer.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <gbVk/ImageView.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Framebuffer {
private:
    VkFramebuffer m_framebuffer;
    VkDevice m_device;
    ImageView m_imageView;
public:
    Framebuffer(VkDevice logical_device, ImageView&& image_view, VkFramebuffer framebuffer);

    ~Framebuffer();

    Framebuffer(Framebuffer const&) = delete;
    Framebuffer& operator=(Framebuffer const&) = delete;

    Framebuffer(Framebuffer&& rhs);
    Framebuffer& operator=(Framebuffer&& rhs) = delete;

    VkFramebuffer getVkFramebuffer();
};
}
#endif
