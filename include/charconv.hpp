#ifndef CHARCONV_HPP
#define CHARCONV_HPP
#include <charconv>
#include <system_error>
#include "type_traits.hpp"
namespace backports{
    enum class chars_format{ scientific = 1, fixed = 2, hex = 4, general = fixed | scientific };
    constexpr chars_format operator|(chars_format l, chars_format r)
    noexcept{ return chars_format(unsigned(l) | unsigned(r)); }
    constexpr chars_format operator&(chars_format l, chars_format r)
    noexcept{ return chars_format(unsigned(l) & unsigned(r)); }
    constexpr chars_format operator^(chars_format l, chars_format r)
    noexcept{ return chars_format(unsigned(l) ^ unsigned(r)); }
    constexpr chars_format operator~(chars_format x) noexcept{ return chars_format(~unsigned(x)); }
    constexpr chars_format& operator|=(chars_format& l, chars_format r)noexcept{ return l = l | r; }
    constexpr chars_format& operator&=(chars_format& l, chars_format r)noexcept{ return l = l & r; }
    constexpr chars_format& operator^=(chars_format& l, chars_format r)noexcept{ return l = l ^ r; }
    struct to_chars_result {
        char* ptr;
        std::errc ec;
        constexpr explicit operator bool() const noexcept { return ec == std::errc{}; }
    };
    struct from_chars_result {
        const char* ptr;
        std::errc ec;
        constexpr explicit operator bool() const noexcept { return ec == std::errc{}; }
    };
    to_chars_result to_chars(char*beg,char*end,         char      value);
    to_chars_result to_chars(char*beg,char*end,  signed char      value);
    to_chars_result to_chars(char*beg,char*end,unsigned char      value);
    to_chars_result to_chars(char*beg,char*end,  signed short     value);
    to_chars_result to_chars(char*beg,char*end,unsigned short     value);
    to_chars_result to_chars(char*beg,char*end,  signed int       value);
    to_chars_result to_chars(char*beg,char*end,unsigned int       value);
    to_chars_result to_chars(char*beg,char*end,  signed long      value);
    to_chars_result to_chars(char*beg,char*end,unsigned long      value);
    to_chars_result to_chars(char*beg,char*end,  signed long long value);
    to_chars_result to_chars(char*beg,char*end,unsigned long long value);
    to_chars_result to_chars(char*beg,char*end,         char      value,int base);
    to_chars_result to_chars(char*beg,char*end,  signed char      value,int base);
    to_chars_result to_chars(char*beg,char*end,unsigned char      value,int base);
    to_chars_result to_chars(char*beg,char*end,  signed short     value,int base);
    to_chars_result to_chars(char*beg,char*end,unsigned short     value,int base);
    to_chars_result to_chars(char*beg,char*end,  signed int       value,int base);
    to_chars_result to_chars(char*beg,char*end,unsigned int       value,int base);
    to_chars_result to_chars(char*beg,char*end,  signed long      value,int base);
    to_chars_result to_chars(char*beg,char*end,unsigned long      value,int base);
    to_chars_result to_chars(char*beg,char*end,  signed long long value,int base);
    to_chars_result to_chars(char*beg,char*end,unsigned long long value,int base);
    to_chars_result to_chars(char*,char*,bool,int=10)=delete;
    to_chars_result to_chars(char*beg,char*end,      float value);
    to_chars_result to_chars(char*beg,char*end,     double value);
    to_chars_result to_chars(char*beg,char*end,long double value);
    to_chars_result to_chars(char*beg,char*end,      float value,chars_format fmt);
    to_chars_result to_chars(char*beg,char*end,     double value,chars_format fmt);
    to_chars_result to_chars(char*beg,char*end,long double value,chars_format fmt);
    to_chars_result to_chars(char*beg,char*end,      float value,chars_format fmt,int precision);
    to_chars_result to_chars(char*beg,char*end,     double value,chars_format fmt,int precision);
    to_chars_result to_chars(char*beg,char*end,long double value,chars_format fmt,int precision);
    from_chars_result from_chars(const char*beg,const char*end,         char     &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,  signed char     &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,unsigned char     &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,  signed short    &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,unsigned short    &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,  signed int      &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,unsigned int      &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,  signed long     &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,unsigned long     &value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,  signed long long&value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,unsigned long long&value,int base=10);
    from_chars_result from_chars(const char*beg,const char*end,      float&value,chars_format fmt=chars_format::general);
    from_chars_result from_chars(const char*beg,const char*end,     double&value,chars_format fmt=chars_format::general);
    from_chars_result from_chars(const char*beg,const char*end,long double&value,chars_format fmt=chars_format::general);
}// namespace backports
#endif // CHARCONV_HPP
