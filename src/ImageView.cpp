#include <gbVk/ImageView.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
ImageView::ImageView(VkDevice logical_device, VkImageView image_view)
    :m_imageView(image_view), m_device(logical_device)
{
}

ImageView::~ImageView()
{
    if(m_imageView) {
        vkDestroyImageView(m_device, m_imageView, nullptr);
    }
}

ImageView::ImageView(ImageView&& rhs)
    :m_imageView(rhs.m_imageView), m_device(rhs.m_device)
{
    rhs.m_imageView = nullptr;
    rhs.m_device = nullptr;
}

VkImageView ImageView::getVkImageView()
{
    return m_imageView;
}
}
