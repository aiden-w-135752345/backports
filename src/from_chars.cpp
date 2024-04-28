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
#include <locale.h>
#include <exception>
#if _GLIBCXX_HAVE_XLOCALE_H
# include <xlocale.h>
#endif
using namespace backports;
static_assert(
	std::numeric_limits<float>::is_iec559&&std::numeric_limits<double>::is_iec559&&
	std::numeric_limits<size_t>::digits>=32,"must be ieee754"
);
__extension__ using uint128_t = __uint128_t;

//#define FASTFLOAT_DEBUG_ASSERT __glibcxx_assert
#include "fast_float.hpp"
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

template<class T>static from_chars_result from_chars_hex(const char* beg, const char* end, T& value){
    using uint_t = std::conditional_t<is_same_v<T, float>, uint32_t,std::conditional_t<is_same_v<T, double>, uint64_t,uint128_t>>;
    if (beg == end)return {beg, std::errc::invalid_argument};
    const char* const orig_beg = beg;
    short sign = 1;if(*beg == '-'){sign=-1;++beg;}
    // Handle "inf", "infinity", "NaN" and variants thereof.
    if(beg != end&&(*beg=='i'||*beg=='I'||*beg =='n'||*beg=='N')){
		if(starts_with_ci<'i','n','f'>(beg, end)){
			beg += "inf"_len;
			if (starts_with_ci<'i','n','i','t','y'>(beg, end))beg += "inity"_len;
			value=sign*std::numeric_limits<T>::infinity();
			return {beg, std::errc{}};
	    }else if (starts_with_ci<'n','a','n'>(beg, end)){
			beg += "nan"_len;
			if (beg != end && *beg == '('){
			const char* const fallback_beg = beg;
			for (;;){
				++beg;
				if (beg == end){beg = fallback_beg;break;}
				if (*beg == ')'){++beg;break;}
				else if (*beg == '_' || digit_parse_table[(unsigned char)*beg] < 36)continue;
				else{beg = fallback_beg;break;}
				}
			}
			value=std::numeric_limits<T>::quiet_NaN();
			return {beg, std::errc{}};
		}
	}
    bool seen_hexit = beg != end && *beg == '0';
    while (beg != end && *beg == '0'){++beg;}

    uint_t mantissa = 0;
    constexpr int mantissa_bits=is_same_v<T, float>?28:is_same_v<T, double>?60:124;
    int mantissa_idx = mantissa_bits;
    int exponent_adjustment = 0;
    bool seen_decimal_point = false;
    for (; beg != end; ++beg){
		if (*beg == '.' && !seen_decimal_point){seen_decimal_point = true;continue;}
		int hexit = digit_parse_table[(unsigned char)*beg];
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
    if (!seen_hexit)return{orig_beg, std::errc::invalid_argument};
    // Parse the written exponent.
    int written_exponent = 0;
    if (beg != end && (*beg == 'p' || *beg == 'P')){
		const char* const fallback_beg = beg;
		++beg;
		if (beg != end && *beg == '+')++beg;
		from_chars_result fcr = from_chars(beg, end, written_exponent, 10);
		if (fcr.ptr == beg){
			beg = fallback_beg;
		}else{
			beg = fcr.ptr;
			if (mantissa != 0 && fcr.ec == std::errc::result_out_of_range)
			return {beg, std::errc::result_out_of_range};
		}
	}
	T val=sign*std::ldexp(T(mantissa),written_exponent+exponent_adjustment-mantissa_bits);
	if((mantissa!=0&&val==0)||std::isinf(val)){return {beg, std::errc::result_out_of_range};}
	value=val;
    return {beg, std::errc{}};
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
		if (result.ec == std::errc{})value = dbl_value;
		return result;
	}
} // namespace backports
#ifdef _GLIBCXX_LONG_DOUBLE_COMPAT
// Make from_chars for 64-bit long double an alias for the overload for double.
extern "C" from_chars_result
_ZN9backports10from_charsEPKcS1_ReNS_12chars_formatE(const char* beg, const char* end,long double& value,chars_format fmt)
__attribute__((alias ("_ZN9backports10from_charsEPKcS1_RdNS_12chars_formatE")));
#endif
template<class T>constexpr static int calculate_free_digits(int base){
	int digits=0;T max=1;
	while(max<=std::numeric_limits<T>::max()/base){++digits;max*=base;}
	if(max==(std::numeric_limits<T>::max()-base+1)/base+1){++digits;};
	return digits;
}
template<class T>constexpr static unsigned char free_digits_data[]={
	calculate_free_digits<T>( 2),calculate_free_digits<T>( 3),calculate_free_digits<T>( 4),
	calculate_free_digits<T>( 5),calculate_free_digits<T>( 6),calculate_free_digits<T>( 7),
	calculate_free_digits<T>( 8),calculate_free_digits<T>( 9),calculate_free_digits<T>(10),
	calculate_free_digits<T>(11),calculate_free_digits<T>(12),calculate_free_digits<T>(13),
	calculate_free_digits<T>(14),calculate_free_digits<T>(15),calculate_free_digits<T>(16),
	calculate_free_digits<T>(17),calculate_free_digits<T>(18),calculate_free_digits<T>(19),
	calculate_free_digits<T>(20),calculate_free_digits<T>(21),calculate_free_digits<T>(22),
	calculate_free_digits<T>(23),calculate_free_digits<T>(24),calculate_free_digits<T>(25),
	calculate_free_digits<T>(26),calculate_free_digits<T>(27),calculate_free_digits<T>(28),
	calculate_free_digits<T>(29),calculate_free_digits<T>(30),calculate_free_digits<T>(31),
	calculate_free_digits<T>(32),calculate_free_digits<T>(33),calculate_free_digits<T>(34),
	calculate_free_digits<T>(35),calculate_free_digits<T>(36)
};
template<class T>constexpr static uint64_t calculate_try_extra(){
	uint64_t try_extra=0;
	for(int base=2;base<=36;base++){
		T max=1;
		int free=calculate_free_digits<T>(base);
		for(int i=0;i<free;i++){max*=base;}
		if(max){try_extra|=(1ULL<<base);}
	}
	return try_extra;
}
template<class T>constexpr static uint64_t try_extra_data=calculate_try_extra<T>();

template<class T>static from_chars_result from_chars_impl(const char* beg, const char* end, T& val,unsigned base){
    using uchar=unsigned char;
	{
		uchar c = digit_parse_table[(uchar)*beg];
        if (c>=base)return {beg,std::errc::invalid_argument};
		for(;beg!=end&&c==0;++beg){
			c = digit_parse_table[(uchar)*beg];
		}
		if(c>=base||c==0){ val=0; return{beg,{}}; }
		val = c;
		if(beg==end){return{beg,{}}; }
	}
	int free_digits = free_digits_data<T>[base-2];
    for (int digit=1;digit<free_digits&&beg!=end; ++digit,++beg){
        const uchar c = digit_parse_table[(uchar)*beg];
        if (c>=base)return {beg,{}};
		val = val * base + c;
    }
	if(beg==end){return {beg,{}};}
	{
		const uchar c = digit_parse_table[(uchar)*beg];
		if(c>=base)return {beg,{}};
		if(try_extra_data<T>&1ULL<<base&&!(
			__builtin_mul_overflow(val, base, &val)||__builtin_add_overflow(val, c, &val)
		)){return {beg,{}};}
	}
	while (++beg!=end&&digit_parse_table[(uchar)*beg] < base){}
	return {beg,std::errc::result_out_of_range};
}
template<class T>static from_chars_result from_chars(const char* beg, const char* end, T& value,int base,std::true_type){
	if(beg==end)return {beg,std::errc::invalid_argument};
	bool negative = *beg == '-';
	if(negative && beg+1==end) return{beg,std::errc::invalid_argument};
	constexpr T maxT=std::numeric_limits<T>::max();
	static_assert(maxT<=std::numeric_limits<unsigned long long>::max(),"Cannot find large enough integer for magnitude");
	using U = std::conditional_t<maxT<=std::numeric_limits<unsigned int>::max(),unsigned int,
    		  std::conditional_t<maxT<=std::numeric_limits<unsigned long>::max(),unsigned long,
			  unsigned long long>>;
	U magnitude = 0;
	from_chars_result res = from_chars_impl(beg+negative, end, magnitude, base);
	if(res.ec==std::errc::invalid_argument){return {beg,std::errc::invalid_argument};};
	if(!res){return res;};
	if(negative){
		constexpr T minT=std::numeric_limits<T>::min();
		constexpr U maxU=maxT;
		static_assert(maxT+minT>0||(
			maxT+minT>=-maxT&&-(maxT+minT)<=std::numeric_limits<U>::max()-maxU
		),"type is too negatively biased.");
		if(magnitude>-(U)minT){return {res.ptr,std::errc::result_out_of_range};}
		else if(magnitude<=maxU){value=-(T)magnitude;}
		else if(magnitude-maxU<=maxU){value=-(T)(magnitude-maxU)-maxT;}
	}else{
		if (magnitude > maxT) return {res.ptr,std::errc::result_out_of_range};
		value = (T)magnitude;
	}
	return res;
}
template<class T>static from_chars_result from_chars(const char* beg, const char* end, T& value,int base,std::false_type){
	if(beg==end)return {beg,std::errc::invalid_argument};
	constexpr T maxT=std::numeric_limits<T>::max();
	using U = std::conditional_t<maxT<=std::numeric_limits<unsigned int>::max(),unsigned int,T>;
	U magnitude = 0;
	from_chars_result res = from_chars_impl(beg, end, magnitude, base);
	if (!res){return res;};
	if (magnitude > maxT)return {res.ptr,std::errc::result_out_of_range};
	value = (T)magnitude;
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