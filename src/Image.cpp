#include <gbVk/Image.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Image::Image(VkDevice logical_device, VkImage image)
    :m_image(image), m_device(logical_device)
{
}

Image::~Image()
{
    if(m_image) { vkDestroyImage(m_device, m_image, nullptr); }
}

Image::Image(Image&& rhs)
    :m_image(rhs.m_image), m_device(rhs.m_device)
{
    rhs.m_image = nullptr;
    rhs.m_device = nullptr;
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
}
