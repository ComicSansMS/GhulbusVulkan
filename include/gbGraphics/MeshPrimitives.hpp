#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_PRIMITIVES_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_PRIMITIVES_HPP

/** @file
*
* @brief Mesh Primitives.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/IndexData.hpp>
#include <gbGraphics/VertexData.hpp>

#include <gbBase/Assert.hpp>

#include <type_traits>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
namespace Primitives
{

template<typename VertexData_T, typename IndexType_T=uint16_t>
class Quad {
public:
    using VertexData = VertexData_T;
    using Format = typename VertexData::Format;
    using Storage = typename VertexData::Storage;
    using IndexType = IndexType_T;
    using IndexData = IndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
private:
    static auto constexpr PositionIndex = Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Position);
    static auto constexpr ColorIndex = Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Color);
    static auto constexpr TextureIndex = Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Texture);
    static_assert(PositionIndex, "Invalid vertex format: Need at least one component with Position semantics.");
public:
    using Position = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Position, Format>;
    using PositionVectorValueType = typename Position::Layout::ValueType;
    VertexData m_vertexData;
    IndexData m_indexData;
public:
    Quad(PositionVectorValueType width, PositionVectorValueType height) {
        m_vertexData.getStorage().resize(4);
        fillVertexData(width, height);
        m_indexData.getStorage().resize(2);
        fillIndexData();
    }

    void fillVertexData(PositionVectorValueType width, PositionVectorValueType height)
    {
        using PositionVec = typename Position::Layout;
        if constexpr (GhulbusMath::VectorTraits::IsVector2<PositionVec>::value) {
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 0) = PositionVec(0,     0);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 1) = PositionVec(width, 0);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 2) = PositionVec(width, height);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 3) = PositionVec(    0, height);
        } else if constexpr (GhulbusMath::VectorTraits::IsVector3<PositionVec>::value) {
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 0) = PositionVec(0,     0,      0);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 1) = PositionVec(width, 0,      0);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 2) = PositionVec(width, height, 0);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 3) = PositionVec(    0, height, 0);
        } else if constexpr (GhulbusMath::VectorTraits::IsVector4<PositionVec>::value) {
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 0) = PositionVec(0,     0,      0, 1);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 1) = PositionVec(width, 0,      0, 1);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 2) = PositionVec(width, height, 0, 1);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 3) = PositionVec(    0, height, 0, 1);
        } else {
            static_assert(false, "Unsupported Position Type");
        }
        if constexpr(TextureIndex) {
            //
            // (0,0)....(1,0)
            //  |          |
            //  |          |
            // (0,1)....(1,1)
            //
            using GhulbusMath::Vector2;
            using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
            using TexT = typename Texture::Layout::ValueType;
            get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, 0) = Vector2<TexT>(0, 1);
            get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, 1) = Vector2<TexT>(1, 1);
            get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, 2) = Vector2<TexT>(1, 0);
            get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, 3) = Vector2<TexT>(0, 0);
        }
    }

    void fillIndexData()
    {
        std::vector<IndexComponent::Triangle<IndexType_T>>& t = m_indexData.getStorage();
        t[0].i1 = 0;  t[0].i2 = 1;  t[0].i3 = 2;
        t[1].i1 = 0;  t[1].i2 = 2;  t[1].i3 = 3;
    }
};

template<typename VertexData_T, typename IndexType_T=uint16_t>
class Grid {
public:
    using VertexData = VertexData_T;
    using Format = typename VertexData::Format;
    using Storage = typename VertexData::Storage;
    using IndexType = IndexType_T;
    using IndexData = IndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
private:
    static auto constexpr PositionIndex = Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Position);
    static auto constexpr ColorIndex = Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Color);
    static auto constexpr TextureIndex = Format::getIndexForSemantics(VertexFormatBase::ComponentSemantics::Texture);
    static_assert(PositionIndex, "Invalid vertex format: Need at least one component with Position semantics.");
public:
    using Position = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Position, Format>;
    using PositionVectorValueType = typename Position::Layout::ValueType;
    VertexData m_vertexData;
    IndexData m_indexData;
public:
    Grid(PositionVectorValueType width, PositionVectorValueType depth, int n_segments_x, int n_segments_z)
    {
        GHULBUS_PRECONDITION_MESSAGE((n_segments_x > 0) && (n_segments_z > 0),
                                     "Number of grid segments must be at least 1 in each dimension.");
        m_vertexData.getStorage().resize((n_segments_x + 1) * (n_segments_z + 1));
        fillVertexData(width, depth, n_segments_x, n_segments_z);
        m_indexData.getStorage().resize(n_segments_x * n_segments_z * 2);
        fillIndexData(n_segments_x, n_segments_z);
    }

    void fillVertexData(PositionVectorValueType width, PositionVectorValueType depth, int n_segments_x, int n_segments_z)
    {
        using PositionVec = typename Position::Layout;
        PositionVec v;
        v.y = 0;
        if constexpr (GhulbusMath::VectorTraits::IsVector4<PositionVec>::value) {
            v.w = 1;
        } else {
            static_assert(GhulbusMath::VectorTraits::IsVector3<PositionVec>::value,
                          "Unsupported Position Type");
        }
        v.x = -width / static_cast<PositionVectorValueType>(2);
        for (int ix = 0; ix <= n_segments_x; ++ix) {
            v.z = -depth / static_cast<PositionVectorValueType>(2);
            for (int iz = 0; iz <= n_segments_z; ++iz) {
                get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, (n_segments_z + 1)*ix + iz) = v;
                v.z += depth / static_cast<PositionVectorValueType>(n_segments_z);
            }
            v.x += width / static_cast<PositionVectorValueType>(n_segments_x);
        }
        if constexpr(TextureIndex) {
            using GhulbusMath::Vector2;
            using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
            using TexT = typename Texture::Layout::ValueType;
            for (int ix = 0; ix <= n_segments_x; ++ix) {
                for (int iz = 0; iz <= n_segments_z; ++iz) {
                    get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, (n_segments_z + 1)*ix + iz) =
                        Vector2<TexT>(static_cast<PositionVectorValueType>(ix) / static_cast<PositionVectorValueType>(n_segments_x),
                                      static_cast<PositionVectorValueType>(iz) / static_cast<PositionVectorValueType>(n_segments_z));
                }
            }
        }
    }

    void fillIndexData(int n_segments_x, int n_segments_z)
    {
        std::vector<IndexComponent::Triangle<IndexType_T>>& t = m_indexData.getStorage();
        for (int ix = 0; ix < n_segments_x; ++ix) {
            for (int iz = 0; iz < n_segments_z; ++iz) {
                int const idx = 2 * (ix*(n_segments_z) + iz);
                GHULBUS_ASSERT((idx >= 0) && (idx + 1 < static_cast<int>(m_indexData.getNumberOfPrimitives())));
                GHULBUS_ASSERT((ix + 1)*(n_segments_z + 1) + iz + 1 < static_cast<int>(m_vertexData.getNumberOfVertices()));
                t[idx].i1 = static_cast<IndexType_T>(ix * (n_segments_z + 1) + iz);
                t[idx].i2 = static_cast<IndexType_T>(ix * (n_segments_z + 1) + iz + 1);
                t[idx].i3 = static_cast<IndexType_T>((ix + 1) * (n_segments_z + 1) + iz);
                t[idx+1].i1 = static_cast<IndexType_T>((ix + 1) * (n_segments_z + 1) + iz);
                t[idx+1].i2 = static_cast<IndexType_T>(ix * (n_segments_z + 1) + iz + 1);
                t[idx+1].i3 = static_cast<IndexType_T>((ix + 1) * (n_segments_z + 1) + iz + 1);
            }
        }
    }
};
}
}
#endif
