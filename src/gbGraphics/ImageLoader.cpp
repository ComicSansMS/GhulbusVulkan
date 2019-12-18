#include <gbGraphics/ImageLoader.hpp>

#include <gbGraphics/Exceptions.hpp>

#include <gbBase/Finally.hpp>
#include <gbBase/UnusedVariable.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

ImageLoader::ImageLoader(char const* filename)
{
    int width, height, n_channels;
    stbi_uc* texture_data = stbi_load(filename, &width, &height, &n_channels, STBI_rgb_alpha);
    GHULBUS_UNUSED_VARIABLE(n_channels);
    if (!texture_data) { GHULBUS_THROW(Exceptions::IOError(), "Error loading texture file."); }
    auto guard_texture_data = Ghulbus::finally([texture_data]() { stbi_image_free(texture_data); });
    m_data.resize(width * height * 4);
    std::memcpy(m_data.data(), texture_data, width * height * 4);
    m_width = static_cast<uint32_t>(width);
    m_height = static_cast<uint32_t>(height);
}

uint32_t ImageLoader::getWidth() const
{
    return m_width;
}

uint32_t ImageLoader::getHeight() const
{
    return m_height;
}

std::byte const* ImageLoader::getData() const
{
    return m_data.data();
}
}
