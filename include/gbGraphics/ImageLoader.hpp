#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_IMAGE_LOADER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_IMAGE_LOADER_HPP

/** @file
*
* @brief Image Loader.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/Image2d.hpp>

#include <gbVk/ForwardDecl.hpp>


namespace GHULBUS_GRAPHICS_NAMESPACE
{
class ImageLoader {
private:
    std::vector<std::byte> m_data;
    uint32_t m_width;
    uint32_t m_height;
public:
    ImageLoader(char const* filename);

    uint32_t getWidth() const;
    uint32_t getHeight() const;
    std::byte const* getData() const;
};
}
#endif
