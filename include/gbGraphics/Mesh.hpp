#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_HPP

/** @file
*
* @brief Triangle Mesh.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/MemoryBuffer.hpp>

#include <gbVk/ForwardDecl.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;
class ObjParser;

class Mesh {
private:
    GhulbusGraphics::MemoryBuffer m_vertexBuffer;
    GhulbusGraphics::MemoryBuffer m_indexBuffer;
public:
    Mesh(GraphicsInstance& instance, ObjParser const& obj);
    ~Mesh();

    GhulbusGraphics::MemoryBuffer& getVertexBuffer();
    GhulbusGraphics::MemoryBuffer& getIndexBuffer();
};
}
#endif
