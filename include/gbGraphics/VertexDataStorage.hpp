#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_VERTEX_DATA_STORAGE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_VERTEX_DATA_STORAGE_HPP

/** @file
*
* @brief Vertex Data Storage.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
template<typename... Ts> struct VertexDataStorage;
template<> struct VertexDataStorage<> {};
template<typename T> struct VertexDataStorage<T> {
    T head;

    VertexDataStorage() = default;
    VertexDataStorage(VertexDataStorage const&) = default;
    VertexDataStorage& operator=(VertexDataStorage const&) = default;
    VertexDataStorage(VertexDataStorage&&) = default;
    VertexDataStorage& operator=(VertexDataStorage&&) = default;

    template <class U, class = std::enable_if_t<!std::is_base_of_v<VertexDataStorage, std::decay_t<U>>>>
    VertexDataStorage(U&& u)
        : head(std::forward<U>(u))
    {}
};
template<typename T, typename... Ts> struct VertexDataStorage<T, Ts...> {
    T head;
    VertexDataStorage<Ts...> tail;

    VertexDataStorage() = default;
    VertexDataStorage(VertexDataStorage const&) = default;
    VertexDataStorage& operator=(VertexDataStorage const&) = default;
    VertexDataStorage(VertexDataStorage&&) = default;
    VertexDataStorage& operator=(VertexDataStorage&&) = default;

    template <class U, class...Us, class = std::enable_if_t<!std::is_base_of_v<VertexDataStorage, std::decay_t<U>>>>
    VertexDataStorage(U&& u, Us&&...tail)
        : head(std::forward<U>(u)), tail(std::forward<Us>(tail)...)
    {}
};
}

namespace std
{
template<typename... Ts>
class tuple_size<GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>> :
    public std::integral_constant<size_t, sizeof...(Ts)>
{};

template<size_t I, typename... Ts>
struct tuple_element<I, GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>> {
    using type = tuple_element_t<I, tuple<Ts...>>;
};

template<size_t I, typename... Ts >
inline constexpr typename std::tuple_element<I, GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>>::type&
get(GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>& v) noexcept {
    if constexpr (I == 0) { return v.head; } else { return get<I-1>(v.tail); }
}

template<size_t I, typename... Ts >
inline constexpr typename std::tuple_element<I, GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>>::type const&
get(GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...> const& v) noexcept {
    if constexpr (I == 0) { return v.head; } else { return get<I-1>(v.tail); }
}

template<size_t I, typename... Ts >
inline constexpr typename std::tuple_element<I, GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>>::type&&
get(GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>&& v) noexcept {
    if constexpr (I == 0) { return std::move(v.head); } else { return get<I-1>(std::move(v.tail)); }
}

template<size_t I, typename... Ts >
inline constexpr typename std::tuple_element<I, GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...>>::type const&&
get(GHULBUS_GRAPHICS_NAMESPACE::VertexDataStorage<Ts...> const&& v) noexcept {
    if constexpr (I == 0) { return std::move(v.head); } else { return get<I-1>(std::move(v.tail)); }
}
}

namespace GHULBUS_GRAPHICS_NAMESPACE
{
using std::get;
}
#endif
