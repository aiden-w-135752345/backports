#ifndef CSTDDEF_HPP
#define CSTDDEF_HPP
#include <cstddef>
namespace backports{
    enum class byte : unsigned char {};
    namespace _cstddef{
        template<class> struct op {};
        template<> struct op<bool> { using type = byte; };
        template<> struct op<char> { using type = byte; };
        template<> struct op<signed char> { using type = byte; };
        template<> struct op<unsigned char> { using type = byte; };
        template<> struct op<wchar_t> { using type = byte; };
        template<> struct op<char16_t> { using type = byte; };
        template<> struct op<char32_t> { using type = byte; };
        template<> struct op<short> { using type = byte; };
        template<> struct op<unsigned short> { using type = byte; };
        template<> struct op<int> { using type = byte; };
        template<> struct op<unsigned int> { using type = byte; };
        template<> struct op<long> { using type = byte; };
        template<> struct op<unsigned long> { using type = byte; };
        template<> struct op<long long> { using type = byte; };
        template<> struct op<unsigned long long> { using type = byte; };
        template<class T>struct op<const T>: op<T> { };
        template<class T>struct op<volatile T>: op<T> { };
        template<class T>struct op<const volatile T>: op<T> { };
        template<class T>using op_t = typename op<T>::type;
    }
    template<class T>constexpr _cstddef::op_t<T>operator<<(byte b, T s) noexcept
        { return (byte)(unsigned char)((unsigned)b << s); }
    template<class T>constexpr _cstddef::op_t<T>operator>>(byte b, T s) noexcept
        { return (byte)(unsigned char)((unsigned)b >> s); }
    constexpr byte operator|(byte l, byte r) noexcept
        { return (byte)(unsigned char)((unsigned)l | (unsigned)r); }
    constexpr byte operator&(byte l, byte r) noexcept
        { return (byte)(unsigned char)((unsigned)l & (unsigned)r); }
    constexpr byte operator^(byte l, byte r) noexcept
        { return (byte)(unsigned char)((unsigned)l ^ (unsigned)r); }
    constexpr byte operator~(byte b) noexcept
        { return (byte)(unsigned char)~(unsigned)b; }
    template<class T>constexpr _cstddef::op_t<T>&operator<<=(byte& b, T s) noexcept
        { return b = b << s; }
    template<class T>constexpr _cstddef::op_t<T>&operator>>=(byte& b, T s) noexcept
        { return b = b >> s; }
    constexpr byte&operator|=(byte& l, byte r) noexcept{ return l = l | r; }
    constexpr byte&operator&=(byte& l, byte r) noexcept{ return l = l & r; }
    constexpr byte&operator^=(byte& l, byte r) noexcept{ return l = l ^ r; }
    template<class T>constexpr T to_integer(_cstddef::op_t<T> b) noexcept{ return T(b); }
}// namespace backports
#endif // CSTDDEF_HPP