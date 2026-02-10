#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_PRIMITIVES_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_MESH_PRIMITIVES_HPP

/** @file
*
* @brief Mesh Primitives.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/GenericIndexData.hpp>
#include <gbGraphics/VertexData.hpp>

#include <gbMath/AABB3.hpp>

#include <gbBase/Assert.hpp>

#include <cmath>
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
    using IndexData = GenericIndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
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
            static_assert(sizeof(PositionVec) == 0, "Unsupported Position Type");
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
    using IndexData = GenericIndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
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

template<typename VertexData_T, typename IndexType_T=uint16_t>
class Box {
public:
    using VertexData = VertexData_T;
    using Format = typename VertexData::Format;
    using Storage = typename VertexData::Storage;
    using IndexType = IndexType_T;
    using IndexData = GenericIndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
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
    Box(GhulbusMath::AABB3<PositionVectorValueType> const& box) {
        m_vertexData.getStorage().resize(24);
        fillVertexData(box.min.to_vector(), box.max.to_vector());
        m_indexData.getStorage().resize(12);
        fillIndexData();
    }

    void fillVertexData(GhulbusMath::Vector3<PositionVectorValueType> const& min,
                        GhulbusMath::Vector3<PositionVectorValueType> const& max)
    {
        using PositionVec = typename Position::Layout;
        if constexpr (GhulbusMath::VectorTraits::IsVector3<PositionVec>::value) {
            // front
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  0) = PositionVec(min.x, min.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  1) = PositionVec(min.x, max.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  2) = PositionVec(max.x, max.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  3) = PositionVec(max.x, min.y, min.z);
            // right
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  4) = PositionVec(max.x, min.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  5) = PositionVec(max.x, max.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  6) = PositionVec(max.x, max.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  7) = PositionVec(max.x, min.y, max.z);
            // back
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  8) = PositionVec(max.x, min.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData,  9) = PositionVec(max.x, max.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 10) = PositionVec(min.x, max.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 11) = PositionVec(min.x, min.y, max.z);
            // left
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 12) = PositionVec(min.x, min.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 13) = PositionVec(min.x, max.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 14) = PositionVec(min.x, max.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 15) = PositionVec(min.x, min.y, min.z);
            // top
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 16) = PositionVec(min.x, max.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 17) = PositionVec(min.x, max.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 18) = PositionVec(max.x, max.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 19) = PositionVec(max.x, max.y, min.z);
            // bottom
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 20) = PositionVec(max.x, min.y, min.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 21) = PositionVec(max.x, min.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 22) = PositionVec(min.x, min.y, max.z);
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, 23) = PositionVec(min.x, min.y, min.z);
        } else {
            static_assert(sizeof(PositionVec) == 0, "Unsupported Position Type");
        }
        if constexpr(TextureIndex) {
            using GhulbusMath::Vector2;
            using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
            using TexT = typename Texture::Layout::ValueType;
            for(int i = 0; i < 6; ++i) {
                int const idx = i*4;
                get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, idx)     = Vector2<TexT>(0, 1);
                get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, idx + 1) = Vector2<TexT>(0, 0);
                get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, idx + 2) = Vector2<TexT>(1, 0);
                get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, idx + 3) = Vector2<TexT>(1, 1);
            }
        }
    }

    void fillIndexData()
    {
        std::vector<IndexComponent::Triangle<IndexType_T>>& t = m_indexData.getStorage();
        for(int i = 0; i < 6; ++i) {
            t[i*2].i1   = static_cast<IndexType_T>(i*4);
            t[i*2].i2   = static_cast<IndexType_T>(i*4+1);
            t[i*2].i3   = static_cast<IndexType_T>(i*4+2);

            t[i*2+1].i1 = static_cast<IndexType_T>(i*4);
            t[i*2+1].i2 = static_cast<IndexType_T>(i*4+2);
            t[i*2+1].i3 = static_cast<IndexType_T>(i*4+3);
        }
    }
};

template<typename VertexData_T, typename IndexType_T=uint16_t>
class OpenCylinder {
public:
    using VertexData = VertexData_T;
    using Format = typename VertexData::Format;
    using Storage = typename VertexData::Storage;
    using IndexType = IndexType_T;
    using IndexData = GenericIndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
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
    OpenCylinder(PositionVectorValueType radius, PositionVectorValueType height, int n_segments, int n_height_segments = 1) {
        GHULBUS_PRECONDITION(n_segments >= 3);
        GHULBUS_PRECONDITION(n_height_segments >= 1);
        m_vertexData.getStorage().resize((n_segments + 1) * (n_height_segments + 1));
        fillVertexData(radius, height, n_segments, n_height_segments);
        m_indexData.getStorage().resize(n_segments * n_height_segments * 2);
        fillIndexData(n_segments, n_height_segments);
    }

    void fillVertexData(PositionVectorValueType radius, PositionVectorValueType height, int n_segments, int n_height_segments)
    {
        using PositionVec = typename Position::Layout;
        using T = PositionVectorValueType;
        PositionVec v;
        if constexpr (GhulbusMath::VectorTraits::IsVector4<PositionVec>::value) {
            v.w = 1;
        }
        int v_index = 0;
        T const height_increment = height / static_cast<T>(n_height_segments);
        for (int h = 0; h <= n_height_segments; ++h) {
            T const current_height = static_cast<T>(h) * height_increment;
            T const PI_2 = 2 * GhulbusMath::traits::Pi<T>::value;
            T const phi_increment = PI_2 * (static_cast<T>(1) / static_cast<T>(n_segments));
            v.y = current_height;
            for (int i = 0; i <= n_segments; ++i) {
                T const phi = (i == n_segments) ? 0 : (static_cast<T>(i) * phi_increment);
                T const sin_phi = radius * std::sin(phi);
                T const cos_phi = radius * std::cos(phi);
                v.x = sin_phi;
                v.z = cos_phi;
                get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, v_index++) = v;
                if constexpr (TextureIndex) {
                    using GhulbusMath::Vector2;
                    using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
                    using TexT = typename Texture::Layout::ValueType;
                    TexT const current_u = static_cast<TexT>(i) / static_cast<TexT>(n_segments);
                    TexT const current_v = static_cast<TexT>(h) / static_cast<TexT>(n_height_segments);
                    get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, v_index - 1) = Vector2<TexT>(current_u, current_v);
                }
            }
        }
    }

    void fillIndexData(int n_segments, int n_height_segments)
    {
        std::vector<IndexComponent::Triangle<IndexType_T>>& t = m_indexData.getStorage();
        int index = 0;
        for (int h = 0; h < n_height_segments; ++h) {
            int const current_height_base = h * (n_segments + 1);
            int const next_height_base = (h + 1) * (n_segments + 1);
            for (int i = 0; i < n_segments; ++i) {
                t[index].i1 = static_cast<IndexType_T>(current_height_base + i);
                t[index].i2 = static_cast<IndexType_T>(current_height_base + i + 1);
                t[index].i3 = static_cast<IndexType_T>(next_height_base + i + 1);
                ++index;
                t[index].i1 = static_cast<IndexType_T>(current_height_base + i);
                t[index].i2 = static_cast<IndexType_T>(next_height_base + i + 1);
                t[index].i3 = static_cast<IndexType_T>(next_height_base + i);
                ++index;
            }
        }
    }
};

template<typename VertexData_T, typename IndexType_T=uint16_t>
class Disc {
public:
    using VertexData = VertexData_T;
    using Format = typename VertexData::Format;
    using Storage = typename VertexData::Storage;
    using IndexType = IndexType_T;
    using IndexData = GenericIndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
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
    Disc(PositionVectorValueType radius, int n_segments) {
        GHULBUS_PRECONDITION(n_segments >= 3);
        m_vertexData.getStorage().resize(n_segments + 1);
        fillVertexData(radius, n_segments);
        m_indexData.getStorage().resize(n_segments);
        fillIndexData(n_segments);
    }

    void fillVertexData(PositionVectorValueType radius, int n_segments)
    {
        using PositionVec = typename Position::Layout;
        using T = PositionVectorValueType;
        PositionVec v;
        if constexpr (GhulbusMath::VectorTraits::IsVector3<PositionVec>::value) {
            v.z = 0;
        }
        if constexpr (GhulbusMath::VectorTraits::IsVector4<PositionVec>::value) {
            v.w = 1;
        }
        int v_index = 0;
        T const PI_2 = 2 * GhulbusMath::traits::Pi<T>::value;
        T const phi_increment = PI_2 * (static_cast<T>(1) / static_cast<T>(n_segments));
        v.x = 0;
        v.y = 0;
        get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, v_index++) = v;
        if constexpr (TextureIndex) {
            using GhulbusMath::Vector2;
            using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
            using TexT = typename Texture::Layout::ValueType;
            get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, 0) = Vector2<TexT>(0.5, 0.5);
        }
        for (int i = 0; i < n_segments; ++i) {
            T const phi = static_cast<T>(i) * phi_increment;
            T const sin_phi = std::sin(phi);
            T const cos_phi = std::cos(phi);
            v.x = radius * sin_phi;
            v.y = radius * cos_phi;
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, v_index++) = v;
            if constexpr (TextureIndex) {
                using GhulbusMath::Vector2;
                using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
                using TexT = typename Texture::Layout::ValueType;
                TexT constexpr half = static_cast<TexT>(0.5);
                TexT const tu = half*sin_phi + half;
                TexT const tv = half*cos_phi + half;
                get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, v_index - 1) = Vector2<TexT>(tu, tv);
            }
        }
    }

    void fillIndexData(int n_segments)
    {
        std::vector<IndexComponent::Triangle<IndexType_T>>& t = m_indexData.getStorage();
        for (int i = 0; i < n_segments - 1; ++i) {
            t[i].i1 = static_cast<IndexType_T>(0);
            t[i].i2 = static_cast<IndexType_T>(i + 2);
            t[i].i3 = static_cast<IndexType_T>(i + 1);
        }
        t[n_segments - 1].i1 = static_cast<IndexType_T>(0);
        t[n_segments - 1].i2 = static_cast<IndexType_T>(1);
        t[n_segments - 1].i3 = static_cast<IndexType_T>(n_segments);
    }
};

template<typename VertexData_T, typename IndexType_T=uint16_t>
class Cone {
public:
    using VertexData = VertexData_T;
    using Format = typename VertexData::Format;
    using Storage = typename VertexData::Storage;
    using IndexType = IndexType_T;
    using IndexData = GenericIndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
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
    Cone(PositionVectorValueType radius, PositionVectorValueType height, int n_segments) {
        GHULBUS_PRECONDITION(n_segments >= 3);
        m_vertexData.getStorage().resize(n_segments + 2);
        fillVertexData(radius, height, n_segments);
        m_indexData.getStorage().resize(n_segments * 2);
        fillIndexData(n_segments);
    }

    void fillVertexData(PositionVectorValueType radius, PositionVectorValueType height, int n_segments)
    {
        using PositionVec = typename Position::Layout;
        using T = PositionVectorValueType;
        PositionVec v;
        if constexpr (GhulbusMath::VectorTraits::IsVector4<PositionVec>::value) {
            v.w = 1;
        }
        int v_index = 0;
        T const PI_2 = 2 * GhulbusMath::traits::Pi<T>::value;
        T const phi_increment = PI_2 * (static_cast<T>(1) / static_cast<T>(n_segments));
        v.x = 0;
        v.y = 0;
        v.z = 0;
        get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, v_index++) = v;
        v.z = height;
        get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, v_index++) = v;
        if constexpr (TextureIndex) {
            using GhulbusMath::Vector2;
            using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
            using TexT = typename Texture::Layout::ValueType;
            get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, 0) = Vector2<TexT>(0.5, 0.5);
            get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, 1) = Vector2<TexT>(0.5, 0.5);
        }
        v.z = 0;
        for (int i = 0; i < n_segments; ++i) {
            T const phi = static_cast<T>(i) * phi_increment;
            T const sin_phi = std::sin(phi);
            T const cos_phi = std::cos(phi);
            v.x = radius * sin_phi;
            v.y = radius * cos_phi;
            get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, v_index++) = v;
            if constexpr (TextureIndex) {
                using GhulbusMath::Vector2;
                using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
                using TexT = typename Texture::Layout::ValueType;
                TexT constexpr half = static_cast<TexT>(0.5);
                TexT const tu = half*sin_phi + half;
                TexT const tv = half*cos_phi + half;
                get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, v_index - 1) = Vector2<TexT>(tu, tv);
            }
        }
    }

    void fillIndexData(int n_segments)
    {
        std::vector<IndexComponent::Triangle<IndexType_T>>& t = m_indexData.getStorage();
        for (int i = 0; i < n_segments - 1; ++i) {
            t[i].i1 = static_cast<IndexType_T>(0);
            t[i].i2 = static_cast<IndexType_T>(i + 2);
            t[i].i3 = static_cast<IndexType_T>(i + 3);
        }
        t[n_segments - 1].i1 = static_cast<IndexType_T>(0);
        t[n_segments - 1].i2 = static_cast<IndexType_T>(n_segments + 1);
        t[n_segments - 1].i3 = static_cast<IndexType_T>(2);
        for (int i = 0; i < n_segments - 1; ++i) {
            t[i + n_segments].i1 = static_cast<IndexType_T>(1);
            t[i + n_segments].i2 = static_cast<IndexType_T>(i + 3);
            t[i + n_segments].i3 = static_cast<IndexType_T>(i + 2);
        }
        t[2 * n_segments - 1].i1 = static_cast<IndexType_T>(1);
        t[2 * n_segments - 1].i2 = static_cast<IndexType_T>(2);
        t[2 * n_segments - 1].i3 = static_cast<IndexType_T>(n_segments + 1);
    }
};

template<typename VertexData_T, typename IndexType_T=uint16_t>
class Sphere {
public:
    using VertexData = VertexData_T;
    using Format = typename VertexData::Format;
    using Storage = typename VertexData::Storage;
    using IndexType = IndexType_T;
    using IndexData = GenericIndexData<IndexFormatBase::PrimitiveTopology::TriangleList, IndexType>;
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
    Sphere(PositionVectorValueType radius, int n_segments_horizontal, int n_segments_vertical) {
        GHULBUS_PRECONDITION(n_segments_horizontal >= 3);
        GHULBUS_PRECONDITION(n_segments_vertical >= 2);
        m_vertexData.getStorage().resize((n_segments_horizontal + 1) * (n_segments_vertical + 2));
        fillVertexData(radius, n_segments_horizontal, n_segments_vertical);
        m_indexData.getStorage().resize(2 * n_segments_horizontal * n_segments_vertical);
        fillIndexData(n_segments_horizontal, n_segments_vertical);
    }

    void fillVertexData(PositionVectorValueType radius, int n_segments_horizontal, int n_segments_vertical)
    {
        using PositionVec = typename Position::Layout;
        using T = PositionVectorValueType;
        PositionVec v;
        if constexpr (GhulbusMath::VectorTraits::IsVector4<PositionVec>::value) {
            v.w = 1;
        }
        int v_index = 0;
        T const PI_1_2 = GhulbusMath::traits::Pi<T>::value / 2;
        T const PI_2 = 2 * GhulbusMath::traits::Pi<T>::value;
        T const theta_increment = GhulbusMath::traits::Pi<T>::value / n_segments_vertical;
        T const phi_increment = PI_2 * (static_cast<T>(1) / static_cast<T>(n_segments_horizontal));
        for (int h = 0; h <= n_segments_vertical; ++h) {
            T const theta = (h * theta_increment) - PI_1_2;
            T const sin_theta = radius * std::sin(theta);
            T const cos_theta = radius * std::cos(theta);
            v.y = sin_theta;
            for (int i = 0; i <= n_segments_horizontal; ++i) {
                T const phi = (i == n_segments_horizontal) ? 0 : (static_cast<T>(i) * phi_increment);
                T const sin_phi = cos_theta * std::sin(phi);
                T const cos_phi = cos_theta * std::cos(phi);
                v.x = cos_phi;
                v.z = sin_phi;
                get<VertexFormatBase::ComponentSemantics::Position>(m_vertexData, v_index++) = v;
                if constexpr (TextureIndex) {
                    using GhulbusMath::Vector2;
                    using Texture = GetComponentBySemantics_t<VertexFormatBase::ComponentSemantics::Texture, Format>;
                    using TexT = typename Texture::Layout::ValueType;
                    TexT const current_u = static_cast<TexT>(i) / static_cast<TexT>(n_segments_horizontal);
                    TexT const current_v = static_cast<TexT>(h) / static_cast<TexT>(n_segments_vertical);
                    get<VertexFormatBase::ComponentSemantics::Texture>(m_vertexData, v_index - 1) = Vector2<TexT>(current_u, current_v);
                }
            }
        }
    }

    void fillIndexData(int n_segments_horizontal, int n_segments_vertical)
    {
        std::vector<IndexComponent::Triangle<IndexType_T>>& t = m_indexData.getStorage();
        int index = 0;
        for (int h = 0; h < n_segments_vertical; ++h) {
            int const current_height_base = h * (n_segments_horizontal + 1);
            int const next_height_base = (h + 1) * (n_segments_horizontal + 1);
            for (int i = 0; i < n_segments_horizontal; ++i) {
                t[index].i1 = static_cast<IndexType_T>(current_height_base + i);
                t[index].i2 = static_cast<IndexType_T>(next_height_base + i + 1);
                t[index].i3 = static_cast<IndexType_T>(current_height_base + i + 1);
                ++index;
                t[index].i1 = static_cast<IndexType_T>(current_height_base + i);
                t[index].i2 = static_cast<IndexType_T>(next_height_base + i);
                t[index].i3 = static_cast<IndexType_T>(next_height_base + i + 1);
                ++index;
            }
        }
    }
};
}
}
#endif
