// modified from libstdc++ code, see 
// https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/src/c%2B%2B17/floating_from_chars.cc
#include "../include/charconv.hpp"
#include "../include/string_view.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iterator>
#include <limits>
#include <locale.h>
#include <memory_resource>
#include <string>
#include "../include/type_traits.hpp"
#include "fast_float.hpp"
using namespace backports;
static_assert(
	std::numeric_limits<float>::is_iec559&&std::numeric_limits<double>::is_iec559&&
	std::numeric_limits<size_t>::digits>=32,"must be ieee754"
);
__extension__ using uint128_t = __uint128_t;
constexpr static const unsigned char digit_parse_table[]={
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	00, 1, 2, 3, 4, 5, 6, 7, 8, 9,99,99,99,99,99,99,
	99,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
	25,26,27,28,29,30,31,32,33,34,35,99,99,99,99,99,
	99,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
	25,26,27,28,29,30,31,32,33,34,35,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,
	99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99};
template<char...prefix,size_t...idx>
static bool starts_with_ci_impl(const char* beg,std::index_sequence<idx...>){
    bool different=false;
	using ignore=int[];
	(void)ignore{(different|=(beg[idx]!=prefix&&beg[idx]!=(prefix^32)))...};
    return !different;
}

template<unsigned char... prefix>
static bool starts_with_ci(const char* beg, const char* end){
    if (beg+sizeof...(prefix)>end)return false;
    return starts_with_ci_impl<prefix...>(beg,std::make_index_sequence<sizeof...(prefix)>{});
}
static constexpr size_t operator""_len(const char*, size_t l)noexcept{ return l; }

template<class T>static std::enable_if_t<is_signed_v<T>,bool>add_overflow(T*r,std::enable_if_t<true,T> x, std::enable_if_t<true,T> y){
#if __has_builtin(__builtin_add_overflow)
	return __builtin_add_overflow(x,y,r);
#else
	if(x>=0&&y>=0&&x>std::numeric_limits<T>::max()-y){return true;}
	if(x<0&&y<0&&x<std::numeric_limits<T>::min()-y){return true;}
	*r=x+y;return false;
#endif
}
template<class T>static std::enable_if_t<is_unsigned_v<T>,bool>add_overflow(T*r,std::enable_if_t<true,T> x, std::enable_if_t<true,T> y){
#if __has_builtin(__builtin_add_overflow)
	return __builtin_add_overflow(x,y,r);
#else
	*r=x+y;
	return x>std::numeric_limits<T>::max()-y;
#endif
}
template<class T>static std::enable_if_t<is_signed_v<T>,bool>mul_overflow(T*r,std::enable_if_t<true,T> x, std::enable_if_t<true,T> y){
#if __has_builtin(__builtin_mul_overflow)
	return __builtin_mul_overflow(x,y,r);
#else
	if((x>1)&&(y>1)&&x>std::numeric_limits<T>::max()/y){return true;}
	if((x<0)&&(y>1)&&x<std::numeric_limits<T>::min()/y){return true;}
	if((x>1)&&(y<0)&&y<std::numeric_limits<T>::min()/x){return true;}
	if((x<0)&&(y<0)&&x<std::numeric_limits<T>::max()/y){return true;}
	*r=x*y;return false;
#endif
}
template<class T>static std::enable_if_t<is_unsigned_v<T>,bool>mul_overflow(T*r,std::enable_if_t<true,T> x, std::enable_if_t<true,T> y){
#if __has_builtin(__builtin_mul_overflow)
	return __builtin_mul_overflow(x,y,r);
#else
	*r=x*y;
	return y&&x>std::numeric_limits<T>::max()/y;
#endif
}
template<class T>static from_chars_result from_chars_hex(const char* beg, const char* end, T& value){
    using uint_t = std::conditional_t<is_same_v<T, float>, uint32_t,std::conditional_t<is_same_v<T, double>, uint64_t,uint128_t>>;
    const char* ptr = beg;
    if (ptr == end)return {ptr, std::errc::invalid_argument};
    short sign = 1;if(*ptr == '-'){sign=-1;++ptr;}
	if (ptr == end)return{beg, std::errc::invalid_argument};
    // Handle "inf", "infinity", "NaN" and variants thereof.
    if(*ptr=='i'||*ptr=='I'||*ptr =='n'||*ptr=='N'){
		if(starts_with_ci<'i','n','f'>(ptr, end)){
			ptr += "inf"_len;
			if (starts_with_ci<'i','n','i','t','y'>(ptr, end))ptr += "inity"_len;
			value=sign*std::numeric_limits<T>::infinity();
			return {ptr, std::errc{}};
	    }else if (starts_with_ci<'n','a','n'>(ptr, end)){
			ptr += "nan"_len;
			if (ptr != end && *ptr == '('){
			const char* const fallback = ptr;
			for (;;){
				++ptr;
				if (ptr == end){ptr = fallback;break;}
				if (*ptr == ')'){++ptr;break;}
				else if (*ptr == '_' || digit_parse_table[static_cast<unsigned char>(*ptr)] < 36)continue;
				else{ptr = fallback;break;}
				}
			}
			value=std::numeric_limits<T>::quiet_NaN();
			return {ptr, std::errc{}};
		}
	}
    uint_t mantissa = 0;
    constexpr int mantissa_bits=is_same_v<T, float>?32:is_same_v<T, double>?64:128;
    int mantissa_idx = mantissa_bits;
    int exponent_adjustment = -mantissa_bits;
    bool seen_hexit = false,seen_decimal_point = false,exponent_out_of_range=false;
    for (; ptr != end; ++ptr){
		if (*ptr == '.' && !seen_decimal_point){seen_decimal_point = true;continue;}
		int hexit = digit_parse_table[static_cast<unsigned char>(*ptr)];
		if (hexit >= 16)break;
		seen_hexit = true;
		if (mantissa==0&&hexit==0){
			if (seen_decimal_point){
				if(exponent_adjustment<4+std::numeric_limits<int>::min()){
					exponent_out_of_range=true;
				}else{exponent_adjustment -= 4;}
			}
			continue;
		}
		if (!seen_decimal_point){
			if(exponent_adjustment>std::numeric_limits<int>::max()-4){
				exponent_out_of_range=true;
			}else{exponent_adjustment += 4;}
		}
		if (mantissa_idx){
			mantissa_idx -= 4;
			mantissa |= uint_t(hexit) << mantissa_idx;
		}else if (hexit){mantissa|=1;}
	}
    if (!seen_hexit)return{beg, std::errc::invalid_argument};
    // Parse the written exponent.
    int written_exponent = 0;
    if (ptr != end && (*ptr == 'p' || *ptr == 'P')){
		const char* const fallback = ptr++;
		from_chars_result fcr;
		if (ptr != end && *ptr == '+'){
			++ptr;
			if(ptr != end&&*ptr != '-'){
				fcr = from_chars(ptr, end, written_exponent, 10);
			}else{
				fcr={ptr,std::errc::invalid_argument};
			}
		}else{
			fcr = from_chars(ptr, end, written_exponent, 10);
		}
		if (fcr.ptr == ptr){
			ptr = fallback;
		}else{
			ptr = fcr.ptr;
			if (mantissa != 0 && fcr.ec == std::errc::result_out_of_range)return fcr;
		}
	}
	exponent_out_of_range|=add_overflow(&exponent_adjustment,written_exponent,exponent_adjustment);
	T val=sign*std::ldexp(T(mantissa),exponent_adjustment);
	if((mantissa!=0&&(val==0||exponent_out_of_range))||std::isinf(val)){return {ptr, std::errc::result_out_of_range};}
	value=val;
    return {ptr, std::errc{}};
}
namespace backports {
	from_chars_result from_chars(const char* beg, const char* end, float& value,chars_format fmt){
		if (fmt == chars_format::hex)return from_chars_hex(beg, end, value);
		else return fast_float::from_chars(beg, end, value, fmt);
	}
	from_chars_result from_chars(const char* beg, const char* end, double& value,chars_format fmt){
		if (fmt == chars_format::hex)return from_chars_hex(beg, end, value);
		else return fast_float::from_chars(beg, end, value, fmt);
	}
	from_chars_result from_chars(const char* beg, const char* end, long double& value,chars_format fmt){
		if (fmt == chars_format::hex)return from_chars_hex(beg, end, value);
		double dbl_value;
		from_chars_result result;
		result = fast_float::from_chars(beg, end, dbl_value, fmt);
		if (result.ec == std::errc{})value = static_cast<long double>(dbl_value);
		return result;
	}
} // namespace backports
#ifdef _GLIBCXX_LONG_DOUBLE_COMPAT
// Make from_chars for 64-bit long double an alias for the overload for double.
extern "C" from_chars_result
_ZN9backports10from_charsEPKcS1_ReNS_12chars_formatE(const char* beg, const char* end,long double& value,chars_format fmt)
__attribute__((alias ("_ZN9backports10from_charsEPKcS1_RdNS_12chars_formatE")));
#endif
template<class T>constexpr static int calc_free_digits(unsigned char base){
	int digits=0;T max=0,threshold=(std::numeric_limits<T>::max()-base+1)/base;
	// max<=threshold is equivalent to bigger_type(max)*base+base-1<=limits::max()
	while(max<=threshold){max=max*base+base-1;++digits;}
	return digits;
}
template<class T>constexpr static int free_digits_data[35]={
	calc_free_digits<T>( 2),calc_free_digits<T>( 3),calc_free_digits<T>( 4),
	calc_free_digits<T>( 5),calc_free_digits<T>( 6),calc_free_digits<T>( 7),
	calc_free_digits<T>( 8),calc_free_digits<T>( 9),calc_free_digits<T>(10),
	calc_free_digits<T>(11),calc_free_digits<T>(12),calc_free_digits<T>(13),
	calc_free_digits<T>(14),calc_free_digits<T>(15),calc_free_digits<T>(16),
	calc_free_digits<T>(17),calc_free_digits<T>(18),calc_free_digits<T>(19),
	calc_free_digits<T>(20),calc_free_digits<T>(21),calc_free_digits<T>(22),
	calc_free_digits<T>(23),calc_free_digits<T>(24),calc_free_digits<T>(25),
	calc_free_digits<T>(26),calc_free_digits<T>(27),calc_free_digits<T>(28),
	calc_free_digits<T>(29),calc_free_digits<T>(30),calc_free_digits<T>(31),
	calc_free_digits<T>(32),calc_free_digits<T>(33),calc_free_digits<T>(34),
	calc_free_digits<T>(35),calc_free_digits<T>(36)
};
template<class T>static from_chars_result from_chars_impl(const char* beg, const char* end, T& val,int base){
    using uchar=unsigned char;
	if(base<2||base>36){return {beg,std::errc::invalid_argument};}
	uchar checked_base=static_cast<uchar>(base);
	{
		uchar c = digit_parse_table[uchar(*beg)];
        if (c>=checked_base)return {beg,std::errc::invalid_argument};
		for(;beg!=end&&c==0;++beg){
			c = digit_parse_table[uchar(*beg)];
		}
		if(c>=checked_base||c==0){ val=0; return{beg,{}}; }
		val = c;
		if(beg==end){return{beg,{}}; }
	}
	int free_digits = free_digits_data<T>[checked_base-2];
    for (int digit=1;digit<free_digits&&beg!=end; ++digit,++beg){
        const uchar c = digit_parse_table[uchar(*beg)];
        if (c>=checked_base)return {beg,{}};
		val = val * checked_base + c;
    }
	if(beg==end){return {beg,{}};}
	{
		const uchar c = digit_parse_table[uchar(*beg)];
		if(c>=checked_base)return {beg,{}};
		if(!(mul_overflow(&val,val,checked_base)||add_overflow(&val,val, c))){return {beg,{}};}
	}
	while (++beg!=end&&digit_parse_table[uchar(*beg)] < checked_base){}
	return {beg,std::errc::result_out_of_range};
}
template<class T>static from_chars_result from_chars(const char* beg, const char* end, T& value,int base,std::true_type){
	if(beg==end)return {beg,std::errc::invalid_argument};
	bool negative = *beg == '-';
	if(negative && beg+1==end) return{beg,std::errc::invalid_argument};
	constexpr T minT=std::numeric_limits<T>::min(),maxT=std::numeric_limits<T>::max();
	static_assert(maxT<=static_cast<unsigned long long>(std::min<T>(minT+maxT,0)-1),"Cannot find large enough integer for magnitude.");
	using Mag = std::conditional_t<maxT<=static_cast<unsigned int>(std::min<T>(minT+maxT,0)-1),unsigned int,
			std::conditional_t<maxT<=static_cast<unsigned long>(std::min<T>(minT+maxT,0)-1),unsigned long,
			unsigned long long>>;
	constexpr Mag maxU=maxT;
	Mag magnitude = 0;
	from_chars_result res = from_chars_impl(beg+negative, end, magnitude, base);
	if(res.ec==std::errc::invalid_argument){return {beg,std::errc::invalid_argument};}
	if(!res){return res;}
	if(negative){
		if(magnitude>-static_cast<Mag>(minT)){return {res.ptr,std::errc::result_out_of_range};}
		else if(magnitude<=maxU){value=-T(magnitude);}
		else if(magnitude-maxU<=maxU){value=-T(magnitude-maxU)-maxT;}
		static_assert(minT+maxT>=0||minT+maxT+maxT>=0,"Type is too negatively biased.");
	}else{
		if (magnitude > maxT) return {res.ptr,std::errc::result_out_of_range};
		value = T(magnitude);
	}
	return res;
}
template<class T>static from_chars_result from_chars(const char* beg, const char* end, T& value,int base,std::false_type){
	if(beg==end)return {beg,std::errc::invalid_argument};
	constexpr T maxT=std::numeric_limits<T>::max();
	using U = std::conditional_t<maxT<=static_cast<unsigned int>(-1),unsigned int,T>;
	U magnitude = 0;
	from_chars_result res = from_chars_impl(beg, end, magnitude, base);
	if (!res){return res;}
	if (magnitude > maxT)return {res.ptr,std::errc::result_out_of_range};
	value = T(magnitude);
	return {res.ptr,{}};
}
namespace backports{
	from_chars_result from_chars(const char*beg,const char*end,         char     &n,int base){return ::from_chars(beg,end,n,base,std::is_signed<char>{});};
	from_chars_result from_chars(const char*beg,const char*end,  signed char     &n,int base){return ::from_chars(beg,end,n,base,std::true_type{});};
	from_chars_result from_chars(const char*beg,const char*end,unsigned char     &n,int base){return ::from_chars(beg,end,n,base,std::false_type{});};
	from_chars_result from_chars(const char*beg,const char*end,  signed short    &n,int base){return ::from_chars(beg,end,n,base,std::true_type{});};
	from_chars_result from_chars(const char*beg,const char*end,unsigned short    &n,int base){return ::from_chars(beg,end,n,base,std::false_type{});};
	from_chars_result from_chars(const char*beg,const char*end,  signed int      &n,int base){return ::from_chars(beg,end,n,base,std::true_type{});};
	from_chars_result from_chars(const char*beg,const char*end,unsigned int      &n,int base){return ::from_chars(beg,end,n,base,std::false_type{});};
	from_chars_result from_chars(const char*beg,const char*end,  signed long     &n,int base){return ::from_chars(beg,end,n,base,std::true_type{});};
	from_chars_result from_chars(const char*beg,const char*end,unsigned long     &n,int base){return ::from_chars(beg,end,n,base,std::false_type{});};
	from_chars_result from_chars(const char*beg,const char*end,  signed long long&n,int base){return ::from_chars(beg,end,n,base,std::true_type{});};
	from_chars_result from_chars(const char*beg,const char*end,unsigned long long&n,int base){return ::from_chars(beg,end,n,base,std::false_type{});};
}//backports
