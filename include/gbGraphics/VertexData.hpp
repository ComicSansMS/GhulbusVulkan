#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_VERTEX_DATA_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_VERTEX_DATA_HPP

/** @file
*
* @brief Vertex Data.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbGraphics/VertexFormat.hpp>
#include <gbGraphics/VertexDataStorage.hpp>

#include <initializer_list>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
template<typename... T_VertexComponents>
class VertexData {
public:
    using Format = VertexFormat<T_VertexComponents...>;
    using Storage = VertexDataStorage<typename T_VertexComponents::Layout...>;
    static_assert(std::is_standard_layout_v<Storage>, "Vertex data must only consist of standard layout types.");
    static_assert(sizeof(Storage) == Format::getStrideStatic(),
                  "Vertex data storage size mismatch. Did you model the padding fields correctly?");
private:
    std::vector<Storage> m_data;
public:
    VertexData() = default;

    VertexData(std::initializer_list<Storage> init)
        : m_data(init)
    {}

    std::vector<Storage>& getStorage() {
        return m_data;
    }

    std::vector<Storage> const& getStorage() const {
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

    Format getFormat() const {
        return Format{};
    }

    uint32_t getNumberOfVertices() const {
        return static_cast<uint32_t>(m_data.size());
    }
};

template<VertexFormatBase::ComponentSemantics Semantics, typename... Ts>
inline constexpr decltype(auto)
get(VertexData<Ts...>& v, std::size_t vertex_index) noexcept {
    size_t constexpr component_index = *VertexData<Ts...>::Format::getIndexForSemantics(Semantics);
    return get<component_index>(v.getStorage()[vertex_index]);
}

template<typename T>
struct VertexDataFromFormat;

template<typename... T_VertexComponents>
struct VertexDataFromFormat<VertexFormat<T_VertexComponents...>> {
    using type = VertexData<T_VertexComponents...>;
};
}
#endif
