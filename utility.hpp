#include <utility>
#ifndef UTILITY_HPP
#define UTILITY_HPP
#ifndef CONST_INLINE
#define CONST_INLINE const static
#endif
namespace backports{
    template<class T>constexpr const T& as_const(T& t) noexcept{return t;}
    template<class T>void as_const(const T&&) = delete;
    struct in_place_t {explicit in_place_t() = default;};
    constexpr CONST_INLINE in_place_t in_place{};
    template<class T>struct in_place_type_t {explicit in_place_type_t() = default;};
    template<class T>constexpr CONST_INLINE in_place_type_t<T> in_place_type{};
    template<std::size_t I>struct in_place_index_t {explicit in_place_index_t() = default;};
    template<std::size_t I> constexpr CONST_INLINE in_place_index_t<I> in_place_index{};
}//namespace backports
#endif //UTILITY_HPP