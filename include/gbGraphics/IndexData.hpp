#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_INDEX_DATA_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_INDEX_DATA_HPP

/** @file
*
* @brief Index Data.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <cstdint>
#include <type_traits>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class IndexFormatBase {
public:
    enum class PrimitiveTopology {
        TriangleList
    };
private:
public:
};

namespace IndexComponent {
template<typename T>
struct Triangle {
    static_assert(std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::uint32_t>,
                  "Only allowed Index types are uint16_t and uint32_t.");
    static constexpr IndexFormatBase::PrimitiveTopology topology = IndexFormatBase::PrimitiveTopology::TriangleList;
    static constexpr size_t indicesPerPrimitive = 3;
    T i1;
    T i2;
    T i3;
};
}

template<IndexFormatBase::PrimitiveTopology Topology, typename T>
class IndexFormat;

template<typename T>
class IndexFormat<IndexFormatBase::PrimitiveTopology::TriangleList, T> {
public:
    using IndexType = IndexComponent::Triangle<T>;
};

template<IndexFormatBase::PrimitiveTopology Topology, typename T>
class IndexData {
public:
    using IndexFormat = IndexFormat<Topology, T>;
    using IndexType = typename IndexFormat::IndexType;
private:
    std::vector<IndexType> m_data;
public:
    IndexData() = default;

    IndexData(std::initializer_list<IndexType> init)
        : m_data(init)
    {}

    std::vector<IndexType>& getStorage() {
        return m_data;
    }

    std::vector<IndexType> const& getStorage() const {
        return m_data;
    }

    auto size() const {
        return m_data.size();
    }

    auto empty() const {
        return m_data.empty();
    }

    std::byte const* data() const {
        return reinterpret_cast<std::byte const*>(m_data.data());
    }

    IndexFormatBase::PrimitiveTopology getTopology() const {
        return IndexType::topology;
    }

    uint32_t getNumberOfPrimitives() const {
        return static_cast<uint32_t>(m_data.size());
    }
};

}
#endif
