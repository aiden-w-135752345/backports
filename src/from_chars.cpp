// modified from libstdc++ code, see 
// https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/src/c%2B%2B17/floating_from_chars.cc
#include "../include/charconv.hpp"
#include "../include/string_view.hpp"
#define _GLIBCXX_USE_CXX11_ABI 1
#include <algorithm>
#include <array>
#include <bit>
#include <iterator>
#include <limits>
#include <string>
#include <memory_resource>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <locale.h>
#include <bits/functexcept.h>
#if _GLIBCXX_HAVE_XLOCALE_H
# include <xlocale.h>
#endif
using namespace backports;
#if !(_GLIBCXX_FLOAT_IS_IEEE_BINARY32 && _GLIBCXX_DOUBLE_IS_IEEE_BINARY64 && __SIZE_WIDTH__ >= 32)
#error "must be ieee754"
#endif
__extension__ using uint128_t = __uint128_t;

#define FASTFLOAT_DEBUG_ASSERT __glibcxx_assert
#include "fast_float.hpp"
constexpr static const unsigned char digit_parse_table[]={63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,0,1,2,3,4,5,6,7,8,9,63,63,63,63,63,63,63,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,63,63,63,63,63,63,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63};
bool starts_with_ci(const char* first, const char* last, string_view prefix){
    constexpr static unsigned char upper_to_lower_table[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255};
    if (last - first < static_cast<ptrdiff_t>(prefix.length()))return false;
    for (const unsigned char pch : prefix){if (upper_to_lower_table[(unsigned char)*(first++)] != pch)return false;}
    return true;
}
template<typename T>from_chars_result from_chars_hex(const char* first, const char* last, T& value){
    using uint_t = std::conditional_t<is_same_v<T, float>, uint32_t,std::conditional_t<is_same_v<T, double>, uint64_t,uint128_t>>;
    if (first == last)return {first, std::errc::invalid_argument};
    const char* const orig_first = first;
    short sign = 1;if(*first == '-'){sign=-1;++first;}
    // Handle "inf", "infinity", "NaN" and variants thereof.
    if(first != last&&(*first=='i'||*first=='I'||*first =='n'||*first=='N')){
		if(starts_with_ci(first, last, "inf"_sv)){
			first += strlen("inf");
			if (starts_with_ci(first, last, "inity"_sv))first += strlen("inity");
			value=sign*std::numeric_limits<T>::infinity();
			return {first, std::errc{}};
	    }else if (starts_with_ci(first, last, "nan")){
			first += strlen("nan");
			if (first != last && *first == '('){
			const char* const fallback_first = first;
			for (;;){
				++first;
				if (first == last){first = fallback_first;break;}
				if (*first == ')'){++first;break;}
				else if (*first == '_' || digit_parse_table[(unsigned char)*first] < 63)continue;
				else{first = fallback_first;break;}
				}
			}
			value=std::numeric_limits<T>::quiet_NaN();
			return {first, std::errc{}};
		}
	}
    bool seen_hexit = false;
    while (first != last && *first == '0'){seen_hexit = true;++first;}

    uint_t mantissa = 0;
    constexpr int mantissa_bits=is_same_v<T, float>?28:is_same_v<T, double>?60:124;
    int mantissa_idx = mantissa_bits;
    int exponent_adjustment = 0;
    bool seen_decimal_point = false;
    for (; first != last; ++first){
		if (*first == '.' && !seen_decimal_point){seen_decimal_point = true;continue;}
		int hexit = digit_parse_table[(unsigned char)*first];
		if (hexit >= 16)break;
		seen_hexit = true;
		if (!seen_decimal_point && mantissa != 0)exponent_adjustment += 4;
		else if (seen_decimal_point && mantissa == 0){
			exponent_adjustment -= 4;
			if (hexit == 0x0)continue;
		}
		if (mantissa_idx >= 0){
			mantissa |= uint_t(hexit) << mantissa_idx;
			mantissa_idx -= 4;
		}else if (hexit){mantissa|=1;}
	}
    if (!seen_hexit)return{orig_first, std::errc::invalid_argument};
    // Parse the written exponent.
    int written_exponent = 0;
    if (first != last && (*first == 'p' || *first == 'P')){
		const char* const fallback_first = first;
		++first;
		if (first != last && *first == '+')++first;
		from_chars_result fcr = from_chars(first, last, written_exponent, 10);
		if (fcr.ptr == first){
			first = fallback_first;
		}else{
			first = fcr.ptr;
			if (mantissa != 0 && fcr.ec == std::errc::result_out_of_range)
			return {first, std::errc::result_out_of_range};
		}
	}
	T val=sign*ldexp(T(mantissa),written_exponent+exponent_adjustment-mantissa_bits);
	if((mantissa!=0&&val==0)||std::isinf(val)){return {first, std::errc::result_out_of_range};}
	value=val;
    return {first, std::errc{}};
}
from_chars_result backports::from_chars(const char* first, const char* last, float& value,chars_format fmt){
	if (fmt == chars_format::hex)return from_chars_hex(first, last, value);
	else return fast_float::from_chars(first, last, value, fmt);
}
from_chars_result backports::from_chars(const char* first, const char* last, double& value,chars_format fmt){
	if (fmt == chars_format::hex)return from_chars_hex(first, last, value);
	else return fast_float::from_chars(first, last, value, fmt);
}
from_chars_result backports::from_chars(const char* first, const char* last, long double& value,chars_format fmt){
  if (fmt == chars_format::hex)return from_chars_hex(first, last, value);
  double dbl_value;
  from_chars_result result;
  result = fast_float::from_chars(first, last, dbl_value, fmt);
  if (result.ec == std::errc{})value = dbl_value;
  return result;
}

#ifdef _GLIBCXX_LONG_DOUBLE_COMPAT
// Make from_chars for 64-bit long double an alias for the overload for double.
extern "C" from_chars_result
_ZN9backports10from_charsEPKcS1_ReNS_12chars_formatE(const char* first, const char* last,long double& value,chars_format fmt)
__attribute__((alias ("_ZN9backports10from_charsEPKcS1_RdNS_12chars_formatE")));
#endif

template<class T>using unsigned_least_t = 
    std::conditional_t<sizeof(T)<sizeof(unsigned int),unsigned int,
    std::conditional_t<sizeof(T)<sizeof(unsigned long),unsigned long,unsigned long long>>;
template<class T>static bool from_chars_impl(const char*& parsed, const char* end, T& uval,unsigned base){
    using uchar=unsigned char;
    int free_digits = __gnu_cxx::__int_traits<T>::__digits/std::__bit_width(base);
    for (; parsed != end; ++parsed){
        const uchar c = digit_parse_table[(uchar)*parsed];
        if (c>=base)return true;
        if (--free_digits >= 0)
            // We're definitely not going to overflow.
            uval = uval * base + c;
        else if (!std::__detail::__raise_and_add(uval, base, c)){
            while (++parsed!=end&&digit_parse_table[(uchar)*parsed] < base);
            return false;
        }
    }
    return true;
}
template<typename T>from_chars_result from_chars(const char* first, const char* last, T& value,int base = 10){
	int sign = 1;
	if (std::is_signed<T>::value&&first != last && *first == '-'){sign = -1;++first;}
	using U = unsigned_least_t<T>;
	U val = 0;
	const char* parsed = first;
	bool valid = from_chars_impl(parsed, last, val, base);
	if (first == parsed)return {parsed,std::errc::invalid_argument};
	if (!valid) return {parsed,std::errc::result_out_of_range};
	if (val > __gnu_cxx::__int_traits<T>::__max) return {parsed,std::errc::result_out_of_range};
	if (std::is_signed<T>::value){
		T tmp;
		if (__builtin_mul_overflow(val, sign, &tmp))return {parsed,std::errc::result_out_of_range};
		value = tmp;
	}else value = val;
	return {parsed,{}};
}
from_chars_result backports::from_chars(const char*beg,const char*end,         char     &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,  signed char     &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,unsigned char     &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,  signed short    &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,unsigned short    &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,  signed int      &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,unsigned int      &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,  signed long     &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,unsigned long     &n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,  signed long long&n,int base){return ::from_chars(beg,end,n,base);};
from_chars_result backports::from_chars(const char*beg,const char*end,unsigned long long&n,int base){return ::from_chars(beg,end,n,base);};
