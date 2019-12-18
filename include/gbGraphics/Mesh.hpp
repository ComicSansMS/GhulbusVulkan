#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_HPP

/** @file
*
* @brief Triangle Mesh.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/Image2d.hpp>
#include <gbGraphics/MemoryBuffer.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <gbMath/Vector2.hpp>
#include <gbMath/Vector3.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class ImageLoader;
class ObjParser;

class Mesh {
public:
    struct VertexData {
        GhulbusMath::Point3f position;              ///< Geometry
        GhulbusMath::Normal3f normal;               ///< Normal
        GhulbusMath::Vector2f texCoord;             ///< Texture Coordinate
    };
    using IndexData = std::uint32_t;
private:
    GhulbusGraphics::MemoryBuffer m_vertexBuffer;
    GhulbusGraphics::MemoryBuffer m_indexBuffer;
    GhulbusGraphics::Image2d m_texture;
public:
    Mesh(GraphicsInstance& instance, ObjParser const& obj, ImageLoader const& texture_loader);
    Mesh(GraphicsInstance& instance, VertexData const* vertex_data, size_t n_vertices,
         IndexData const* index_data, size_t n_indices, ImageLoader const& texture_loader);

    uint32_t getNumberOfIndices() const;
    uint32_t getNumberOfVertices() const;

    GhulbusGraphics::MemoryBuffer& getVertexBuffer();
    GhulbusGraphics::MemoryBuffer& getIndexBuffer();
    GhulbusGraphics::Image2d& getTexture();
};
}
#endif
