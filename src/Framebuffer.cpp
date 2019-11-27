#include <gbVk/Framebuffer.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

Framebuffer::Framebuffer(VkDevice logical_device, ImageView&& image_view, VkFramebuffer framebuffer)
    :m_framebuffer(framebuffer), m_device(logical_device), m_imageView(std::move(image_view))
{
}

Framebuffer::~Framebuffer()
{
    if (m_framebuffer) {
        vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
    }
}

Framebuffer::Framebuffer(Framebuffer&& rhs)
    :m_framebuffer(rhs.m_framebuffer), m_device(rhs.m_device), m_imageView(std::move(rhs.m_imageView))
{
    rhs.m_framebuffer = nullptr;
    rhs.m_device = nullptr;
}

VkFramebuffer Framebuffer::getVkFramebuffer()
{
    return m_framebuffer;
}
}
