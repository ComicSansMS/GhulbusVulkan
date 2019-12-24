#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_VERTEX_FORMAT_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_VERTEX_FORMAT_HPP

/** @file
*
* @brief Vertex Format.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbMath/Vector2.hpp>
#include <gbMath/Vector3.hpp>
#include <gbMath/Vector4.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
struct VertexComponentInfo;

class VertexFormatBase {
public:
    enum class ComponentType {
        Padding,
        t_bool,
        t_int,
        t_uint,
        t_float,
        t_double,
        t_bvec2,
        t_bvec3,
        t_bvec4,
        t_ivec2,
        t_ivec3,
        t_ivec4,
        t_uvec2,
        t_uvec3,
        t_uvec4,
        t_vec2,
        t_vec3,
        t_vec4,
        t_dvec2,
        t_dvec3,
        t_dvec4,
        t_mat2,
        t_mat3,
        t_mat4,
        t_mat2x2,
        t_mat3x2,
        t_mat4x2,
        t_mat2x3,
        t_mat3x3,
        t_mat4x3,
        t_mat2x4,
        t_mat3x4,
        t_mat4x4
    };

    enum class ComponentSemantics {
        None,
        Generic,
        Padding,
        Position,
        Color,
        Normal,
        Texture
    };

public:
    virtual ~VertexFormatBase() = default;

    VertexFormatBase(VertexFormatBase const&) = delete;
    VertexFormatBase& operator=(VertexFormatBase const&) = delete;

    std::unique_ptr<VertexFormatBase> clone() const {
        return do_clone();
    }

    size_t getStride() const {
        return do_getStride();
    }

    size_t getNumberOfComponents() const {
        return do_getNumberOfComponents();
    }

    VertexComponentInfo const& getVertexComponentInfo(size_t index) {
        return do_getVertexComponentInfo(index);
    }

    size_t getComponentSize(size_t index) const;
    size_t getComponentOffset(size_t index) const;
    ComponentType getComponentType(size_t index) const;
    ComponentSemantics getComponentSemantics(size_t index) const;
protected:
    VertexFormatBase() = default;
    virtual std::unique_ptr<VertexFormatBase> do_clone() const = 0;
    virtual size_t do_getStride() const = 0;
    virtual size_t do_getNumberOfComponents() const = 0;
    virtual VertexComponentInfo const& do_getVertexComponentInfo(size_t index) const = 0;
};

struct VertexComponentInfo {
    std::size_t size;
    std::size_t offset;
    VertexFormatBase::ComponentType type;
    VertexFormatBase::ComponentSemantics semantics;
};

inline size_t VertexFormatBase::getComponentSize(size_t index) const {
    return do_getVertexComponentInfo(index).size;
}

inline size_t VertexFormatBase::getComponentOffset(size_t index) const {
    return do_getVertexComponentInfo(index).offset;
}

inline VertexFormatBase::ComponentType VertexFormatBase::getComponentType(size_t index) const {
    return do_getVertexComponentInfo(index).type;
}

inline VertexFormatBase::ComponentSemantics VertexFormatBase::getComponentSemantics(size_t index) const {
    return do_getVertexComponentInfo(index).semantics;
}

namespace VertexComponentLayout {
    template<size_t N>
    struct Padding {
        std::byte padding[N];
    };
}

template<size_t N>
constexpr std::integral_constant<VertexFormatBase::ComponentType, VertexFormatBase::ComponentType::Padding>
getVertexComponentType(VertexComponentLayout::Padding<N> const&);

constexpr std::integral_constant<VertexFormatBase::ComponentType, VertexFormatBase::ComponentType::t_vec2>
getVertexComponentType(GhulbusMath::Vector2f const&);

constexpr std::integral_constant<VertexFormatBase::ComponentType, VertexFormatBase::ComponentType::t_vec3>
getVertexComponentType(GhulbusMath::Vector3f const&);

constexpr std::integral_constant<VertexFormatBase::ComponentType, VertexFormatBase::ComponentType::t_vec4>
getVertexComponentType(GhulbusMath::Vector4<float> const&);

template<typename T>
struct InvalidLayout {
    static constexpr VertexFormatBase::ComponentType invalid() {
        static_assert(sizeof(T) == 0, "Type is not a valid Vertex Component Layout.");
    }
    static constexpr VertexFormatBase::ComponentType value = invalid();
};
template<typename T>
inline constexpr InvalidLayout<T> getVertexComponentType(T);


namespace VertexComponentSemantics {
    struct None {};

    template<int = 0>
    struct Generic {};

    struct Padding {};

    struct Position {};

    struct Color {};

    struct Normal {};

    struct Texture {};
}

constexpr std::integral_constant<VertexFormatBase::ComponentSemantics, VertexFormatBase::ComponentSemantics::None>
getVertexComponenentSemantics(VertexComponentSemantics::None);

template<int N>
constexpr std::integral_constant<VertexFormatBase::ComponentSemantics, VertexFormatBase::ComponentSemantics::Generic>
getVertexComponenentSemantics(VertexComponentSemantics::Generic<N>);

constexpr std::integral_constant<VertexFormatBase::ComponentSemantics, VertexFormatBase::ComponentSemantics::Padding>
getVertexComponenentSemantics(VertexComponentSemantics::Padding);

constexpr std::integral_constant<VertexFormatBase::ComponentSemantics, VertexFormatBase::ComponentSemantics::Position>
getVertexComponenentSemantics(VertexComponentSemantics::Position);

constexpr std::integral_constant<VertexFormatBase::ComponentSemantics, VertexFormatBase::ComponentSemantics::Color>
getVertexComponenentSemantics(VertexComponentSemantics::Color);

constexpr std::integral_constant<VertexFormatBase::ComponentSemantics, VertexFormatBase::ComponentSemantics::Normal>
getVertexComponenentSemantics(VertexComponentSemantics::Normal);

constexpr std::integral_constant<VertexFormatBase::ComponentSemantics, VertexFormatBase::ComponentSemantics::Texture>
getVertexComponenentSemantics(VertexComponentSemantics::Texture);

template<typename T>
struct InvalidSemantic {
    static constexpr VertexFormatBase::ComponentSemantics invalid() {
        static_assert(sizeof(T) == 0, "Type is not a valid Vertex Component Semantic.");
    }
    static constexpr VertexFormatBase::ComponentSemantics value = invalid();
};
template<typename T>
inline constexpr InvalidSemantic<T> getVertexComponenentSemantics(T);

template<typename T_Layout, typename T_Semantics = VertexComponentSemantics::None>
struct VertexComponent {
    using layout_type = T_Layout;
    using semantics_type = T_Semantics;
};

namespace detail {

template<typename T_VertexComponent>
static constexpr VertexComponentInfo determineComponentInfo() {
    using layout = typename T_VertexComponent::layout_type;
    using semantics = typename T_VertexComponent::semantics_type;
    return VertexComponentInfo{
        sizeof(layout),
        0,                  // offset will be filled in a second step
        decltype(getVertexComponentType(std::declval<layout>()))::value,
        decltype(getVertexComponenentSemantics(std::declval<semantics>()))::value
    };
}

}

template<typename... T_VertexComponents>
class VertexFormat : public VertexFormatBase {
private:
    std::array<VertexComponentInfo, sizeof...(T_VertexComponents)> m_runtimeInfo;
public:
    VertexFormat()
        :m_runtimeInfo(buildRuntimeInfo())
    {}

    ~VertexFormat() override = default;
protected:
    std::unique_ptr<VertexFormatBase> do_clone() const override {
        return std::make_unique<VertexFormat>();
    }

    size_t do_getStride() const override {
        return m_runtimeInfo.back().offset + m_runtimeInfo.back().size;
    }
    size_t do_getNumberOfComponents() const override {
        return sizeof...(T_VertexComponents);
    }
    VertexComponentInfo const& do_getVertexComponentInfo(size_t index) const override {
        return m_runtimeInfo[index];
    }

    static constexpr std::array<VertexComponentInfo, sizeof...(T_VertexComponents)> buildRuntimeInfo()
    {
        std::array<VertexComponentInfo, sizeof...(T_VertexComponents)> ret{ detail::determineComponentInfo<T_VertexComponents>()... };
        std::size_t acc_offset = 0;
        for (auto& info : ret) { info.offset = acc_offset; acc_offset += info.size; }
        return ret;
    }
};

}
#endif
