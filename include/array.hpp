#ifndef ARRAY_HPP
#define ARRAY_HPP
#include <array>
namespace backports{
    template<class C>constexpr auto size(const C& c) -> decltype(c.size()){return c.size();}
    template<class T, std::size_t N>constexpr std::size_t size(const T (&)[N]) noexcept{return N;}

    template<class C>using _ssize_t=std::common_type_t<std::ptrdiff_t,std::make_signed_t<decltype(std::declval<C>().size())>>;
    
    template<class C>constexpr _ssize_t<C> ssize(const C& c){return static_cast<_ssize_t<C>>(c.size());}
    template<class T, std::ptrdiff_t N>constexpr std::ptrdiff_t ssize(const T (&)[N]) noexcept{return N;}

    template<class C>constexpr auto empty(const C& c) -> decltype(c.empty()){return c.empty();}
    template<class T, std::size_t N>constexpr bool empty(const T (&)[N]) noexcept{return false;}
    template<class E>constexpr bool empty(std::initializer_list<E> il) noexcept{return il.size() == 0;}

    template<class C>constexpr auto data(C& c) -> decltype(c.data()){return c.data();}
    template<class C>constexpr auto data(const C& c) -> decltype(c.data()){return c.data();}
    template<class T, std::size_t N>constexpr T* data(T (&array)[N]) noexcept{return array;}
    template<class E>constexpr const E* data(std::initializer_list<E> il) noexcept{return il.begin();}
}// namespace backports
#endif // ARRAY_HPP