#include <gbVk/ImageView.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

ImageView::ImageView(VkDevice logical_device, VkImageView image_view)
    :m_image_view(image_view), m_device(logical_device)
{
}

ImageView::~ImageView()
{
    if(m_image_view) {
        vkDestroyImageView(m_device, m_image_view, nullptr);
    }
}

ImageView::ImageView(ImageView&& rhs)
    :m_image_view(rhs.m_image_view), m_device(rhs.m_device)
{
    rhs.m_image_view = nullptr;
    rhs.m_device = nullptr;
}

VkImageView ImageView::getVkImageView()
{
    return m_image_view;
}
}
