// modified from libstdc++ code, see 
// https://gcc.gnu.org/git/?p=gcc.git;a=blob;f=libstdc%2B%2B-v3/src/c%2B%2B17/floating_to_chars.cc
#include "../include/charconv.hpp"
#include "../include/optional.hpp"
#include "../include/string_view.hpp"
#include "to_chars_tables.hpp"
#include <bit>
#include <cfenv>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#if __has_include(<langinfo.h>)
#include <langinfo.h> // for nl_langinfo
#endif
#include <type_traits>
#include <cstdint>
using namespace backports;
template<class T>struct ieee_t{
	typename floating_type_traits<T>::mantissa_t mantissa;
	int32_t exponent;
	bool negative;
};
// Decompose the floating-point value into its IEEE components.
template<class T>ieee_t<T>get_ieee_repr(T value){
	constexpr int mantissa_bits = floating_type_traits<T>::mantissa_bits;
	using mantissa_t=typename floating_type_traits<T>::mantissa_t;
	int32_t exponent;
	T mantissa=std::abs(std::ldexp(std::frexp(value,&exponent),mantissa_bits+1));
	return ieee_t<T>{static_cast<mantissa_t>(mantissa),exponent,std::signbit(value)};
}

namespace ryu{
	int to_chars(const floating_decimal_32 v, char* const result);
	int to_chars(const floating_decimal_64 v, char* const result);
	int to_chars(const floating_decimal_128 v, char* const result);
	int d2fixed_buffered_n(double d, uint32_t precision, char* result) ;
	int d2exp_buffered_n(double d, uint32_t precision, char* result, int* exp_out) ;
	floating_decimal_64 floating_to_fd(double f);
	floating_decimal_32 floating_to_fd(float f);
	uint32_t decimalLength(const uint32_t v);
	uint32_t decimalLength(const uint64_t v);
	uint32_t decimalLength(const uint128_t v);
	floating_decimal_128 generic_binary_to_decimal(
		const uint128_t ieeeMantissa, const uint32_t ieeeExponent, const bool ieeeSign,
		const uint32_t mantissaBits, const uint32_t exponentBits, const bool explicitLeadingBit);

	// Invoke Ryu to obtain the shortest scientific form for the given
	// floating-point number.
	static floating_decimal_128 floating_to_fd(long double value){
		ieee_t<long double>repr=get_ieee_repr(value);
		using Traits=floating_type_traits<long double>;
		return ryu::generic_binary_to_decimal(
			repr.mantissa,uint32_t(repr.exponent+(int32_t(1)<<(Traits::exponent_bits-1))-2),
			repr.negative,Traits::mantissa_bits,Traits::exponent_bits,0);
	}

} // namespace ryu
template<class T>constexpr static std::pair<T,short>calc_max_pow10(){
    short digits=1;
	T max=1,threshold=std::numeric_limits<T>::max()/10;
	while(max<=threshold){max*=10;++digits;}
	return {max,digits};
}
template<class T>constexpr static std::pair<T,short>max_pow10=calc_max_pow10<T>();
template<class T>static short width_10(T value){
    T p10 = max_pow10<T>.first;
    for (short i = max_pow10<T>.second; i > 0; i--) {
      if (value >= p10) {return i;}
      p10 /= 10;
    }
    return 1;
}
template<class T>static backports::to_chars_result to_chars_10(char* beg, char* end, T val){
	constexpr T maxT=std::numeric_limits<T>::max();
	using U = std::conditional_t<maxT<=static_cast<unsigned int>(-1),unsigned int,
		std::conditional_t<maxT<=static_cast<unsigned long>(-1),unsigned long,
		std::conditional_t<maxT<=static_cast<unsigned long long>(-1),unsigned long long,uint128_t>>>;
	
	U uval = U(val);
	if(val < 0){++beg;uval=-uval;}
	short len=width_10(uval);
	if ((end - beg) < len){return {end,std::errc::value_too_large};}
	end=beg+len;
	while(len){beg[--len]='0'+(uval%10);uval/=10;}
	if(val < 0){beg[-1]='-';}
	return {end,{}};
}
template<class T>static int count_trailing_zero(T value){
	// if(!value)UB;
	int c=0;
	value&=-value;
	for(int i=std::numeric_limits<T>::digits/2;i;i>>=1){if(value&~((~T(0))/((T(1)<<i)+1))){c+=i;}}
	return c;
}
#if __has_builtin(__builtin_ctz)
static int count_trailing_zero(unsigned char value){return __builtin_ctz(value);}
static int count_trailing_zero(unsigned short value){return __builtin_ctz(value);}
static int count_trailing_zero(unsigned int value){return __builtin_ctz(value);}
#endif
#if __has_builtin(__builtin_ctzl)
static int count_trailing_zero(unsigned long value){return __builtin_ctzl(value);}
#endif
#if __has_builtin(__builtin_ctzll)
static int count_trailing_zero(unsigned long long value){return __builtin_ctzll(value);}
static int count_trailing_zero(uint128_t value){
	unsigned long long low = value & static_cast<unsigned long long>(-1);
	if (low != 0)return count_trailing_zero(low);
	constexpr auto N = std::numeric_limits<unsigned long long>::digits;
	unsigned long long high = value >> N;
	return count_trailing_zero(high) + N;
}
#endif
template<class T>static int bit_width(T value){
    int r=0;
    for(int s=std::numeric_limits<T>::digits/2;s;s>>=1){if(value&((~T(0))<<(s-1))){value>>=s;r+=s;}}
    return r;
}
#if __has_builtin(__builtin_clz)
static int bit_width(unsigned char value){return std::numeric_limits<unsigned int>::digits-__builtin_clz(value);}
static int bit_width(unsigned short value){return std::numeric_limits<unsigned int>::digits-__builtin_clz(value);}
static int bit_width(unsigned int value){return std::numeric_limits<unsigned int>::digits-__builtin_clz(value);}
#endif
#if __has_builtin(__builtin_clzl)
static int bit_width(unsigned long value){return std::numeric_limits<unsigned long>::digits-__builtin_clzl(value);}
#endif
#if __has_builtin(__builtin_clzll)
static int bit_width(unsigned long long value){return std::numeric_limits<unsigned long long>::digits-__builtin_clzll(value);}
static int bit_width(uint128_t value){
        constexpr auto N = std::numeric_limits<unsigned long long>::digits;
		unsigned long long high = value >> N;
		if (high != 0){return N+bit_width(high);}
		unsigned long long low = value & static_cast<unsigned long long>(-1);
		return bit_width(low);
}
#endif
constexpr static char digits[]="0123456789abcdefghijklmnopqrstuvwxyz";
template<unsigned char bits,class T>static backports::to_chars_result to_chars_bin(char* beg, char* end, T val){
	unsigned char len=static_cast<unsigned char>((bit_width(val|1)+bits-1)/bits);
	if ((end - beg) < len){return {end,std::errc::value_too_large};}
	end=beg+len;
	while(len){beg[--len]=digits[val&((1<<bits)-1)];val>>=bits;}
	return {end,{}};
}
template<class T>static backports::to_chars_result to_chars(char* beg, char* end, T val, int base){
	if(base<2||base>36){return {end,std::errc::value_too_large};}
	unsigned char checked_base=static_cast<unsigned char>(base);
	if(checked_base==10){return to_chars_10(beg,end,val);}
	constexpr T maxT=std::numeric_limits<T>::max();
	using U = std::conditional_t<maxT<=static_cast<unsigned int>(-1),unsigned int,
		std::conditional_t<maxT<=static_cast<unsigned long>(-1),unsigned long,
		std::conditional_t<maxT<=static_cast<unsigned long long>(-1),unsigned long long,uint128_t>>>;
	if(beg == end) return { end, std::errc::value_too_large };
	U uval = U(val);
	if(val < 0){*beg++ = '-';uval=-uval;}
	switch(checked_base){
		case 2:return to_chars_bin<1>(beg,end,uval);case 4:return to_chars_bin<2>(beg,end,uval);
		case 8:return to_chars_bin<3>(beg,end,uval);
		case 16:return to_chars_bin<4>(beg,end,uval);case 32:return to_chars_bin<5>(beg,end,uval);
		default:break;
	}
	unsigned char len=1;for(U lval=uval;lval>=checked_base;lval/=checked_base){len++;}
	if ((end - beg) < len){return {end,std::errc::value_too_large};}
	end=beg+len;
	while(len){beg[--len]=digits[uval%checked_base];uval/=checked_base;}
	return {end,{}};
}
namespace backports{
	to_chars_result to_chars(char*beg,char*end,         char      val,int base){return ::to_chars(beg,end,+val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed char      val,int base){return ::to_chars(beg,end,+val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned char      val,int base){return ::to_chars(beg,end,+val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed short     val,int base){return ::to_chars(beg,end,+val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned short     val,int base){return ::to_chars(beg,end,+val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed int       val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned int       val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed long      val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned long      val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed long long val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned long long val,int base){return ::to_chars(beg,end,val,base);}

	to_chars_result to_chars(char*beg,char*end,         char      val){return ::to_chars_10(beg,end,+val);}
	to_chars_result to_chars(char*beg,char*end,  signed char      val){return ::to_chars_10(beg,end,+val);}
	to_chars_result to_chars(char*beg,char*end,unsigned char      val){return ::to_chars_10(beg,end,+val);}
	to_chars_result to_chars(char*beg,char*end,  signed short     val){return ::to_chars_10(beg,end,+val);}
	to_chars_result to_chars(char*beg,char*end,unsigned short     val){return ::to_chars_10(beg,end,+val);}
	to_chars_result to_chars(char*beg,char*end,  signed int       val){return ::to_chars_10(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned int       val){return ::to_chars_10(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,  signed long      val){return ::to_chars_10(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned long      val){return ::to_chars_10(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,  signed long long val){return ::to_chars_10(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned long long val){return ::to_chars_10(beg,end,val);}
}
// This subroutine returns true if the shortest scientific form fd is a
// positive power of 10, and the floating-point number that has this shortest
// scientific form is smaller than this power of 10.
//
// For instance, the exactly-representable 64-bit number
// 99999999999999991611392.0 has the shortest scientific form 1e23, so its
// exact value is smaller than its shortest scientific form.
//
// For these powers of 10 the length of the fixed form is one digit less
// than what the scientific exponent suggests.
//
// This subroutine inspects a lookup table to detect when fd is such a
// "rounded up" power of 10.
template<class T>static bool is_rounded_up_pow10_p(typename floating_type_traits<T>::shortest_scientific_t fd){
	if (fd.exponent < 0 || fd.mantissa != 1) return false;
	return(floating_type_traits<T>::pow10_adjustment_tab[fd.exponent/64]&(1ull<<(63-fd.exponent%64)));
}
static constexpr int operator""_len(const char*, size_t l)noexcept{ return int(l); }
// This subroutine handles writing nan, inf and 0 in
// all formatting modes.
template<class T> static optional<to_chars_result>to_chars_special(char* first, char* last, T value,chars_format fmt, int precision){
	bool negative = std::signbit(value);
	switch(std::fpclassify(value)){
	case FP_INFINITE:
		if (last-first < negative + "inf"_len)
			return {{last, std::errc::value_too_large}};
		if (negative)*first++ = '-';
		memcpy(first, "inf", "inf"_len);
		return {{first + "inf"_len, {}}};
	break;
	case FP_NAN:
		if (last-first < negative + "nan"_len)
			return {{last, std::errc::value_too_large}};
		if (negative)*first++ = '-';
		memcpy(first, "nan", "nan"_len);
		return {{first + "nan"_len, {}}};
	break;
	case FP_ZERO:break;
	default:case FP_SUBNORMAL:case FP_NORMAL:return nullopt;
	}
	// We're formatting 0.
	switch (fmt){
	case chars_format::fixed:{
		if (last-first < negative + "0"_len + (precision>0?"."_len + precision:0))
			return {{last, std::errc::value_too_large}};
		if (negative)*first++ = '-';
		*first++ = '0';
		if (precision>0){*first++ = '.';memset(first, '0', size_t(precision));first += precision;}
		return {{first,{}}};
		}

	case chars_format::scientific:{
		if (last-first < negative + "0"_len + (precision>0?"."_len + precision:0) + "e+00"_len)
			return {{last, std::errc::value_too_large}};
		if (negative)*first++ = '-';
		*first++ = '0';
		if (precision>0){*first++ = '.';memset(first, '0', size_t(precision));first += precision;}
		memcpy(first, "e+00", 4);first += 4;
		return {{first,{}}};
	}
	case chars_format::hex:{
		if (last-first < negative + "0"_len + (precision>0?"."_len + precision:0) + "p+0"_len)
			return {{last, std::errc::value_too_large}};
		if (negative)*first++ = '-';
		*first++ = '0';
		if (precision>0){*first++ = '.';memset(first, '0', size_t(precision));first += precision;}
		memcpy(first, "p+0", 3);first += 3;
		return {{first,{}}};
	}
	case chars_format::general:
		if (last-first < negative+"0"_len) return {{last, std::errc::value_too_large}};
		if (negative)*first++ = '-';
		*first++ = '0';
		return {{first,{}}};
	}
}
// This subroutine of the floating-point to_chars overloads performs
// hexadecimal formatting.
template<class T>static to_chars_result to_chars_hex(char* first, char* last, T value,int precision){
	constexpr int mantissa_bits = floating_type_traits<T>::mantissa_bits;
	using mantissa_t = typename floating_type_traits<T>::mantissa_t;
	if (auto result = to_chars_special(first, last, value,chars_format::hex,precision))return *result;
	// Extract the sign, mantissa and exponent from the value.
	ieee_t<T> repr=get_ieee_repr(value);
	// Compute the shortest precision needed to print this value exactly,
	// disregarding trailing zeros.
	int shortest_full_precision = (mantissa_bits-count_trailing_zero(repr.mantissa) + 3) / 4;
	if(precision<0){precision=shortest_full_precision;}
	if(precision < shortest_full_precision){
		// When limiting the precision, we need to determine how to round the
		// least significant printed hexit. The following branchless
		// bit-level-parallel technique computes whether to round up the
		// mantissa bit at index N (according to round-to-nearest rules) when
		// dropping N bits of precision, for each index N in the bit vector.
		// This technique is borrowed from the MSVC implementation.
		mantissa_t round_bit = repr.mantissa << 1, lsb_hexit=(mantissa_t{1}<<mantissa_bits)>>(4*precision);
		mantissa_t has_tail_bits = round_bit - 1, lsb_bit = repr.mantissa;
		mantissa_t should_round = round_bit & (has_tail_bits | lsb_bit);
		// Mask out the dropped nibbles.
		repr.mantissa&=~(lsb_hexit-1);
		repr.mantissa+=(should_round&lsb_hexit);
	}
	// Now before we start writing the string, determine the total length of
	// the output string and perform a single bounds check.
	int abs_exponent = abs(repr.exponent-1);
	if (last-first < repr.negative + "d"_len + (precision>0?"."_len:0) + precision + (
		abs_exponent >= 10000 ? "p+ddddd"_len :
		abs_exponent >= 1000 ? "p+dddd"_len :
		abs_exponent >= 100 ? "p+ddd"_len :
		abs_exponent >= 10 ? "p+dd"_len : "p+d"_len
	)) return {last, std::errc::value_too_large};
	// Write the negative sign and the leading hexit.
	if (repr.negative)*first++ = '-';
	*first++ = '0' + char(repr.mantissa>>mantissa_bits);
	if (precision > 0){
		*first++ = '.';
		// Write the rest of the mantissa
		int hexits=0;
		while (repr.mantissa&((mantissa_t{1}<<mantissa_bits)-1)){
			repr.mantissa<<=4;
			first[hexits++] = "0123456789abcdef"[0xf&int(repr.mantissa>>mantissa_bits)];
		}
		memset(first+hexits, '0', size_t(precision-hexits));
		first+=precision;
	}
	// Finally, write the exponent.
	*first++ = 'p';
	if (repr.exponent-1 >= 0)*first++ = '+';
	return to_chars_10(first, last, repr.exponent-1);
}
template<class... Args> static int snprintf_tonearest(char* buffer,int maxlen,const char* format, Args... args){
	int mode=fegetround();fesetround(FE_TONEAREST);
	int len = snprintf(buffer,size_t(maxlen+1),format, args...);
	fesetround(mode);
	return len;
}
/*
static inline uint32_t mulShift0_mod1e9(const uint64_t m, const uint64_t* const mul, const int32_t j) {
	const uint128_t v= (((uint128_t) m * mul[1]) + (uint64_t) (((uint128_t) m * mul[0]) >> 64)) >> j;
	//const uint128_t v0=((uint128_t) m * 3906250u) >> j;
	//const uint128_t v1=(((uint128_t) m * 59u) + (uint64_t) (((uint128_t) m * 11153727427136454656u) >> 64)) >> j;
	//const uint128_t v2=((uint64_t) (((uint128_t) m * 16777216000000000u) >> 64)) >> j;
	//const uint128_t v3=((uint64_t) (((uint128_t) m * 256000000000u) >> 64)) >> j;

	uint128_t hi;
	const uint64_t aLo = (uint64_t)v;
	const uint32_t aHi = (uint64_t)(v >> 64); 
	const uint128_t b00 = (uint128_t)aLo * 0x31680A88F8953031u;
	const uint128_t b01 = (uint128_t)aLo * 0x89705F4136B4A597u;
	const uint128_t b10 = (uint128_t)aHi * 0x31680A88F8953031u;
	const uint64_t b11 = aHi * 0x89705F4136B4A597u;
	const uint128_t mid1 = b10 + (uint64_t)(b00 >> 64);
	const uint64_t multiplied = b11 + (uint64_t)(mid1 >> 64)+(uint64_t)((b01 + (uint64_t)(mid1)) >> 64);
	// For uint32_t truncation, see the mod1e9() comment in d2s_intrinsics.h.
	return ((uint32_t) v) - 1000000000 * (uint32_t) (multiplied >> 29);
}
int d2fixed_buffered_n(double d, char* result) {
  const uint64_t bits = double_to_bits(d);
  // Decode bits into sign, mantissa, and exponent.
  const bool ieeeSign = ((bits >> (DOUBLE_MANTISSA_BITS + DOUBLE_EXPONENT_BITS)) & 1) != 0;
  const uint64_t ieeeMantissa = bits & ((1ull << DOUBLE_MANTISSA_BITS) - 1);
  const int32_t ieeeExponent = (int32_t) ((bits >> DOUBLE_MANTISSA_BITS) & ((1u << DOUBLE_EXPONENT_BITS) - 1)) - DOUBLE_BIAS;

  int32_t e2 = ieeeExponent - DOUBLE_MANTISSA_BITS;
  uint64_t m2 = (1ull << DOUBLE_MANTISSA_BITS) | ieeeMantissa;
  int index = 0;
  bool zero = true;
  if (ieeeSign) {result[index++] = '-';}
  if (ieeeExponent >= 0) {
    const uint32_t idx = e2 < 0 ? 0 : indexForExponent((uint32_t) e2);
    const uint32_t p10bits = pow10BitsForIndex(idx);
    const int32_t len = (int32_t) lengthForIndex(idx);
    for (int32_t i = len - 1; i >= 0; --i) {
      const uint32_t j = p10bits - e2;
      // Temporary: j is usually around 128, and by shifting a bit, we push it to 128 or above, which is
      // a slightly faster code path in mulShift_mod1e9. Instead, we can just increase the multipliers.
      const uint32_t digits = mulShift_mod1e9(m2 << 8, POW10_SPLIT[POW10_OFFSET[idx] + i], (int32_t) (j + 8));
      if (!zero) {
        append_nine_digits(digits, result + index);
        index += 9;
      } else if (digits != 0) {
        const uint32_t olength = decimalLength(digits);
        append_n_digits(olength, digits, result + index);
        index += olength;
        zero = false;
      }
    }
  }
  if (zero) {result[index++] = '0';}
const uint64_t table[4][2] = {
  {                   0u,              3906250u },
  {11153727427136454656u,                   59u },
  {   16777216000000000u,                    0u },
  {        256000000000u,                    0u },
  };
  if (ieeeExponent < DOUBLE_MANTISSA_BITS) {
    const int32_t idx = (DOUBLE_MANTISSA_BITS-ieeeExponent) / 16;// 0...3
    // 0 = don't round up; 1 = round up unconditionally; 2 = round up if odd.
    int roundUp = 0;
    uint32_t digits = mulShift0_mod1e9(m2 << 8, table[idx], (DOUBLE_MANTISSA_BITS-ieeeExponent)%16);
    uint32_t lastDigit = 0;
    for (uint32_t k = 0; k < 9; ++k) {
      lastDigit = digits % 10;
      digits /= 10;
    }
    if (lastDigit != 5) {
      roundUp = lastDigit > 5;
    } else {
      // Is m * 10^(additionalDigits + 1) / 2^(-e2) integer?
      roundUp = multipleOfPowerOf2(m2, (uint32_t) (DOUBLE_MANTISSA_BITS-ieeeExponent - 1) ) ? 2 : 1;
    }
    if (roundUp != 0) {
      int roundIndex = index;
      int dotIndex = 0; // '.' can't be located at index 0
      while (true) {
        --roundIndex;
        char c;
        if (roundIndex == -1 || (c = result[roundIndex], c == '-')) {
          result[roundIndex + 1] = '1';
          if (dotIndex > 0) {
            result[dotIndex] = '0';
            result[dotIndex + 1] = '.';
          }
          result[index++] = '0';
          break;
        }
        if (c == '.') {
          dotIndex = roundIndex;
          continue;
        } else if (c == '9') {
          result[roundIndex] = '0';
          roundUp = 1;
          continue;
        } else {
          if (roundUp == 2 && c % 2 == 0) {
            break;
          }
          result[roundIndex] = c + 1;
          break;
        }
      }
    }
  }
  return index;
}
*/
template<class T>static to_chars_result to_chars_short(char* first, char* last, T value,chars_format fmt,typename floating_type_traits<T>::shortest_scientific_t fd,int mantissa_length){
	if (fmt == chars_format::scientific){
		// Calculate the total length of the output string, perform a bounds
		// check, and then defer to Ryu's to_chars subroutine.
		int abs_exponent = abs(fd.exponent + mantissa_length - 1);
		if (last-first < fd.sign + mantissa_length+ (mantissa_length > 1?"."_len:0) + (
			abs_exponent >= 1000 ? "e+dddd"_len :
			abs_exponent >= 100 ? "e+ddd"_len : "e+dd"_len
		))return {last, std::errc::value_too_large};
		return {first + ryu::to_chars(fd, first),{}};
	}else if (fmt == chars_format::fixed && fd.exponent >= 0){
		// The Ryu exponent is positive, and so this number's shortest
		// representation is a whole number, to be formatted in fixed instead
		// of scientific notation "as if by std::printf". This means we may
		// need to print more digits of the IEEE mantissa than what the
		// shortest scientific form given by Ryu provides.
		//
		// For instance, the exactly representable number
		// 12300000000000001048576.0 has as its shortest scientific
		// representation 123e+22, so in this case fd.mantissa is 123 and
		// fd.exponent is 22, which doesn't have enough information to format
		// the number exactly. So we defer to Ryu's d2fixed_buffered_n with
		// precision=0 to format the number in the general case here.

		// To that end, first compute the output length and perform a bounds
		// check.
		int expected_output_length = fd.sign + mantissa_length + fd.exponent;
		if (is_rounded_up_pow10_p<T>(fd))--expected_output_length;
		if (last-first < expected_output_length)return {last, std::errc::value_too_large};

		// Optimization: if the shortest representation fits inside the IEEE
		// mantissa, then the number is certainly exactly-representable and
		// its shortest scientific form must be equal to its exact form. So
		// we can write the value in fixed form exactly via fd.mantissa and
		// fd.exponent.
		//
		// Taking log2 of both sides of the desired condition
		//   fd.mantissa * 10^fd.exponent < 2^mantissa_bits
		// we get
		//   log2 fd.mantissa + fd.exponent * log2 10 < mantissa_bits
		// where log2 10 is slightly smaller than 10/3=3.333...
		//
		// After adding some wiggle room due to rounding we get the condition
		// value_fits_inside_mantissa_p below.
		int log2_mantissa = bit_width(fd.mantissa) - 1;
		bool value_fits_inside_mantissa_p
			= (log2_mantissa + (fd.exponent*10 + 2) / 3 < floating_type_traits<T>::mantissa_bits - 2);
		if (value_fits_inside_mantissa_p){
			// Print the small exactly-representable number in fixed form by
			// writing out fd.mantissa followed by fd.exponent many 0s.
			if (fd.sign)*first++ = '-';
			to_chars_result result = to_chars_10(first, last, fd.mantissa);
			memset(result.ptr, '0', size_t(fd.exponent));
			result.ptr += fd.exponent;
			return result;
		}else if (is_same_v<T, long double>){
			// We can't use d2fixed_buffered_n for types larger than double,
			// so we instead format larger types through sprintf.
			// TODO: We currently go through an intermediate buffer in order
			// to accommodate the mandatory null terminator of sprintf, but we
			// can avoid this if we use sprintf to write all but the last
			// digit, and carefully compute and write the last digit
			// ourselves.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-extension"
			char buffer[expected_output_length + 1];
#pragma clang diagnostic pop
			int output_length = snprintf_tonearest(buffer,expected_output_length,"%.0Lf", double(value));
			memcpy(first, buffer, size_t(output_length));
			return {first + output_length,{}};
		}else{
			// Otherwise, the number is too big, so defer to d2fixed_buffered_n.
			int output_length = ryu::d2fixed_buffered_n(double(value), 0, first);
			return {first + output_length,{}};
		}
	}else if (fmt == chars_format::fixed && fd.exponent < 0){
		// The Ryu exponent is negative, so fd.mantissa definitely contains
		// all of the whole part of the number, and therefore fd.mantissa and
		// fd.exponent contain all of the information needed to format the
		// number in fixed notation "as if by std::printf" (with precision
		// equal to -fd.exponent).
		int whole_digits = std::max<int>(mantissa_length + fd.exponent, 1);
		if (last-first < fd.sign + whole_digits + "."_len - fd.exponent)return {last, std::errc::value_too_large};
		if (mantissa_length <= -fd.exponent){
			// The magnitude of the number is less than one.  Format the
			// number appropriately.
			if (fd.sign)*first++ = '-';
			*first++ = '0';
			*first++ = '.';
			int leading_zeros = -fd.exponent - mantissa_length;
			memset(first, '0', size_t(leading_zeros));
			first += leading_zeros;
			return to_chars_10(first, last, fd.mantissa);
		}else{
			// The magnitude of the number is at least one.
			if (fd.sign)*first++ = '-';
			to_chars_result result = to_chars_10(first, last, fd.mantissa);
			// Make space for and write the decimal point in the correct spot.
			memmove(&result.ptr[fd.exponent+1], &result.ptr[fd.exponent],size_t(-fd.exponent));
			result.ptr[fd.exponent] = '.';
			return result;
		}
	}
	__builtin_unreachable();
}
template<class T>static to_chars_result to_chars_plain(char* first, char* last, T value){
	if (auto result = to_chars_special(first, last, value, chars_format::general, 0))return *result;
	auto fd = ryu::floating_to_fd(value);
	int mantissa_length = int(ryu::decimalLength(fd.mantissa));
	// The 'plain' formatting mode resolves to 'scientific' if it yields
	// the shorter string, and resolves to 'fixed' otherwise. The
	// following lower and upper bounds on the exponent characterize when
	// to prefer 'fixed' over 'scientific'.
	int lower_bound = -(mantissa_length + 3);
	int upper_bound = 5;
	if (mantissa_length == 1){
		// The decimal point in scientific notation will be omitted in this
		// case; tighten the bounds appropriately.
		++lower_bound;--upper_bound;
	}
	return to_chars_short(
		first,last,value,
		(lower_bound <= fd.exponent  && fd.exponent <= upper_bound)?chars_format::fixed:chars_format::scientific,
		fd,mantissa_length
	);

};
template<class T>static to_chars_result to_chars_fmt(char* first, char* last, T value,chars_format fmt){
	if (fmt == chars_format::hex){return to_chars_hex(first, last, value, -1);}
	if (auto result = to_chars_special(first, last, value, fmt, 0))return *result;
	auto fd = ryu::floating_to_fd(value);
	int mantissa_length = int(ryu::decimalLength(fd.mantissa));
	if (fmt == chars_format::general){
		// Resolve the 'general' formatting mode as per the specification of
		// the 'g' printf output specifier. Since there is no precision
		// argument, the default precision of the 'g' specifier, 6, applies.
		int scientific_exponent = fd.exponent + mantissa_length - 1;
		if (-4<=scientific_exponent && scientific_exponent < 6)fmt = chars_format::fixed;
		else fmt = chars_format::scientific;
	}
	return to_chars_short(first,last,value,fmt,fd,mantissa_length);
}
template<class T>static to_chars_result to_chars_prec(char* first, char* last, T value,chars_format fmt, int precision){
	if (fmt == chars_format::hex)return to_chars_hex(first, last, value, precision);
	if (precision < 0)precision=6;
	if (auto result = to_chars_special(first, last, value,fmt, precision))return *result;
	constexpr int mantissa_bits = floating_type_traits<T>::mantissa_bits;
	// Extract the sign and exponent from the value.
	ieee_t<T> repr=get_ieee_repr(value);
	int exp_sign=(repr.exponent>0)-(repr.exponent<0);
	// this is below log10(abs(value))
	// 21306/70777 is slightly less than log10 2
	//int lower_log10_value = (repr.exponent-exp_sign)*21306/70777;
	// this is above log10(abs(value))
	// 4004/13301 is slightly greater than log10 2.
	int upper_log10_value = repr.exponent*4004/13301+exp_sign;
	// This is within +-0.651 of log10(abs(value)).
	// 8008/26602 is an approximation of log10 2, 9297/26602 is an approximation of (1-log10 2)/2
	//int approx_log10_value = (repr.exponent*8008 + exp_sign*9297)/26602;
	// Compute (an upper bound of) the number's effective precision when it is
	// formatted in scientific and fixed notation.  Beyond this precision all
	// digits are definitely zero, and this fact allows us to bound the sizes
	// of any local output buffers that we may need to use.  TODO: Consider
	// the number of trailing zero bits in the mantissa to obtain finer upper
	// bounds.
	// ???: Using "mantissa_bits + 1" instead of just "mantissa_bits" in the
	// bounds below is necessary only for __ibm128, it seems.  Even though the
	// type has 105 bits of precision, printf may output 106 fractional digits
	// on some inputs, e.g. 0x1.bcd19f5d720d12a3513e3301028p+0.
	int max_eff_scientific_precision
		= (repr.exponent >= 0
		? std::max(mantissa_bits + 1, upper_log10_value)
		: -(7*repr.exponent + 9)/10 + 2 + mantissa_bits + 1);
	int max_eff_fixed_precision
		= (repr.exponent >= 0
		? mantissa_bits + 1
		: -repr.exponent + mantissa_bits + 1);
	// Ryu doesn't support formatting floating-point types larger than double
	// with an explicit precision, so instead we just go through printf.
	if (is_same_v<T, long double>){
		int effective_precision;
		const char* output_specifier;
		if (fmt == chars_format::scientific){
			effective_precision = std::min(precision, max_eff_scientific_precision);
			output_specifier = "%.*Le";
		}else if (fmt == chars_format::fixed){
			effective_precision = std::min(precision, max_eff_fixed_precision);
			output_specifier = "%.*Lf";
		}else if (fmt == chars_format::general){
			effective_precision = std::min(precision, max_eff_scientific_precision);
			output_specifier = "%.*Lg";
		}else __builtin_unreachable();
		int excess_precision = (fmt != chars_format::general? precision - effective_precision : 0);
		// Since the output of printf is locale-sensitive, we need to be able
		// to handle a radix point that's different from '.'.
		char radix_buf[10];
		string_view radix = effective_precision?string_view(radix_buf+1,size_t(snprintf(radix_buf,10,"%.1f", 0.0)-2)):"."_sv;
		// Compute straightforward upper bounds on the output length.
		int output_length_upper_bound;
		if (fmt == chars_format::fixed){
			if (repr.exponent >= 0) output_length_upper_bound = repr.negative + upper_log10_value;
			else output_length_upper_bound = repr.negative + "0"_len + int(radix.length()) + effective_precision;
		}else{output_length_upper_bound = "-d"_len+int(radix.length())+effective_precision+"e+dddd"_len;}
		// Do the sprintf into the local buffer.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-extension"
		char buffer[output_length_upper_bound + 1];
#pragma clang diagnostic pop
		int output_length = snprintf_tonearest(buffer,output_length_upper_bound, output_specifier,effective_precision,double(value));
		if (effective_precision > 0){
			// We need to replace a radix that is different from '.' with '.'.
			if (radix != "."){
				size_t radix_index = string_view(buffer, size_t(output_length)).find(radix);
				if (radix_index != string_view::npos){
					buffer[radix_index] = '.';
					if (radix.length() > 1){
						memmove(&buffer[radix_index + 1],
							&buffer[radix_index + radix.length()],
							size_t(output_length) - radix_index - radix.length());
						output_length -= radix.length() - 1;
					}
				}
			}
		}
		// Copy the string from the buffer over to the output range.
		if (last-first < output_length+excess_precision)
			return {last, std::errc::value_too_large};
		memcpy(first, buffer, size_t(output_length));
		first += output_length;

		// Add the excess 0s to the result.
		if (excess_precision > 0){
			if (fmt == chars_format::scientific){
				char* significand_end
				= (output_length >= 6 && first[-6] == 'e' ? &first[-6]
					: first[-5] == 'e' ? &first[-5]
					: &first[-4]);
				memmove(significand_end + excess_precision, significand_end,
					size_t(first - significand_end));
				memset(significand_end, '0', size_t(excess_precision));
				first += excess_precision;
			}else if (fmt == chars_format::fixed){
				memset(first, '0', size_t(excess_precision));
				first += excess_precision;
			}
		}
		return {first,{}};
	}else if (fmt == chars_format::scientific){
		int effective_precision= std::min(precision, max_eff_scientific_precision);
		int excess_precision = precision - effective_precision;
		// We can easily compute the output length exactly whenever the
		// scientific exponent is far enough away from +-100. But if it's
		// near +-100, then our log2 approximation is too coarse (and doesn't
		// consider precision-dependent rounding) in order to accurately
		// distinguish between a scientific exponent of +-100 and +-99.
		// Compute an upper bound on the output length. TODO: Maybe also
		// consider a lower bound on the output length.
		int output_length_upper_bound = repr.negative + "d"_len+ (effective_precision > 0?"."_len + effective_precision:0);
		if (abs(repr.exponent)>=328)output_length_upper_bound += "e+ddd"_len;
		else output_length_upper_bound += "e+dd"_len;
		int output_length;
		if (last-first >= output_length_upper_bound + excess_precision){
			// The result will definitely fit into the output range, so we can
			// write directly into it.
			output_length = ryu::d2exp_buffered_n(double(value), uint32_t(effective_precision),first, nullptr);
		}else if (abs(abs(repr.exponent) - 332) <= 4){
			// Write the result of d2exp_buffered_n into an intermediate
			// buffer, do a bounds check, and copy the result into the output
			// range.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-extension"
			char buffer[output_length_upper_bound];
#pragma clang diagnostic pop
			output_length = ryu::d2exp_buffered_n(double(value), uint32_t(effective_precision),buffer, nullptr);
			if (last-first < output_length+excess_precision)
				return {last, std::errc::value_too_large};
			memcpy(first, buffer, size_t(output_length));
		}else
			// If the scientific exponent is not near 100, then the upper bound
			// is actually the exact length, and so the result will definitely
			// not fit into the output range.
			return {last, std::errc::value_too_large};
		first += output_length;
		if (excess_precision > 0){
			// Splice the excess zeros into the result.
			char* significand_end = (first[-5] == 'e'? &first[-5] : &first[-4]);
			memmove(significand_end + excess_precision, significand_end,size_t(first - significand_end));
			memset(significand_end, '0', size_t(excess_precision));
			first += excess_precision;
		}
		return {first,{}};
	}else if (fmt == chars_format::fixed){
		int effective_precision = std::min(precision, max_eff_fixed_precision);
		int excess_precision = precision - effective_precision;

		// Compute an upper bound on the output length.  TODO: Maybe also
		// consider a lower bound on the output length.
		int output_length_upper_bound;
		if (repr.exponent >= 0)output_length_upper_bound = repr.negative + upper_log10_value;
		else output_length_upper_bound = repr.negative + "0"_len;
		if (effective_precision > 0)output_length_upper_bound += "."_len + effective_precision;
		int output_length;
		if (last-first >= output_length_upper_bound + excess_precision){
			// The result will definitely fit into the output range, so we can
			// write directly into it.
			output_length = ryu::d2fixed_buffered_n(double(value), uint32_t(effective_precision),first);
		}else{
			// Write the result of d2fixed_buffered_n into an intermediate
			// buffer, do a bounds check, and copy the result into the output
			// range.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-extension"
			char buffer[output_length_upper_bound];
#pragma clang diagnostic pop
			output_length = ryu::d2fixed_buffered_n(double(value), uint32_t(effective_precision),buffer);
			if (last-first < output_length+excess_precision )
				return {last, std::errc::value_too_large};
			memcpy(first, buffer, size_t(output_length));
		}
		first += output_length;
		if (excess_precision > 0){
			// Append the excess zeros into the result.
			memset(first, '0', size_t(excess_precision));
			first += excess_precision;
		}
		return {first,{}};
	}else if (fmt == chars_format::general){
		// Handle the 'general' formatting mode as per C11 printf's %g output
		// specifier. Since Ryu doesn't do zero-trimming, we always write to
		// an intermediate buffer and manually perform zero-trimming there
		// before copying the result over to the output range.
		int effective_precision = std::min(precision, max_eff_scientific_precision + 1);
		int output_length_upper_bound = "-d."_len + effective_precision + "e+ddd"_len;
		// The four bytes of headroom is to avoid needing to do a memmove when
		// rewriting a scientific form such as 1.00e-2 into the equivalent
		// fixed form 0.001.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvla-extension"
		char buffer[4 + output_length_upper_bound];
#pragma clang diagnostic pop
		// 7.21.6.1/8: "Let P equal ... 1 if the precision is zero."
		if (effective_precision == 0)effective_precision = 1;

		// Perform a trial formatting in scientific form, and obtain the
		// scientific exponent.
		int scientific_exponent;
		char* buffer_start = buffer + 4;
		int output_length = ryu::d2exp_buffered_n(double(value),uint32_t(effective_precision - 1),buffer_start, &scientific_exponent);
		// 7.21.6.1/8: "Then, if a conversion with style E would have an
		// exponent of X:
		//   if P > X >= -4, the conversion is with style f and
		//	 precision P - (X + 1).
		//   otherwise, the conversion is with style e and precision P - 1."
		bool resolve_to_fixed_form = (-4<= scientific_exponent && scientific_exponent < effective_precision);
		if (resolve_to_fixed_form){
			// Rather than invoking d2fixed_buffered_n to reformat the number
			// for us from scratch, we can just rewrite the scientific form
			// into fixed form in-place.  This is safe to do because whenever
			// %g resolves to %f, the fixed form will be no larger than the
			// corresponding scientific form, and it will also contain the
			// same significant digits as the scientific form.
			fmt = chars_format::fixed;
			if (scientific_exponent < 0){
				// e.g. buffer_start == "-1.234e-04"
				char* leading_digit = &buffer_start[repr.negative];
				leading_digit[1] = leading_digit[0];
				// buffer_start == "-11234e-04"
				buffer_start -= -scientific_exponent;
				// buffer_start == "????-11234e-04"
				char* head = buffer_start;
				if (repr.negative)*head++ = '-';
				*head++ = '0';
				*head++ = '.';
				memset(head, '0', size_t(-scientific_exponent - 1));
				// buffer_start == "-0.00011234e-04"

				// Now drop the exponent suffix, and add the leading zeros to
				// the output length.
				output_length -= "e-0d"_len;
				output_length += -scientific_exponent;
				if (effective_precision - 1 == 0)
					// The scientific form had no decimal point, but the fixed
					// form now does.
					output_length += "."_len;
			}else if (effective_precision == 1){
				// The scientific exponent must be 0, so the fixed form
				// coincides with the scientific form (minus the exponent
				// suffix).
				output_length -= "e+dd"_len;
			}else{
				// We are dealing with a scientific form which has a
				// non-empty fractional part and a nonnegative exponent,
				// e.g. buffer_start == "1.234e+02".
				char* decimal_point = &buffer_start[repr.negative + 1];
				memmove(decimal_point, decimal_point+1,size_t(scientific_exponent));
				// buffer_start == "123.4e+02"
				decimal_point[scientific_exponent] = '.';
				if (scientific_exponent >= 100)
					output_length -= "e+ddd"_len;
				else
					output_length -= "e+dd"_len;
				if (effective_precision - 1 == scientific_exponent)
					output_length -= "."_len;
			}
			effective_precision -= 1 + scientific_exponent;

		}else{
			// We're sticking to the scientific form, so keep the output as-is.
			fmt = chars_format::scientific;
			effective_precision = effective_precision - 1;
		}
		// 7.21.6.1/8: "Finally ... any any trailing zeros are removed from
		// the fractional portion of the result and the decimal-point
		// character is removed if there is no fractional portion remaining."
		if (effective_precision > 0){
			char* decimal_point = nullptr;
			if (fmt == chars_format::scientific)decimal_point = &buffer_start[repr.negative + 1];
			else if (fmt == chars_format::fixed)decimal_point
			= &buffer_start[output_length] - effective_precision - 1;
			char* fractional_part_start = decimal_point + 1;
			char* fractional_part_end = nullptr;
			if (fmt == chars_format::scientific){
				fractional_part_end = (buffer_start[output_length-5] == 'e'
							? &buffer_start[output_length-5]
							: &buffer_start[output_length-4]);
			}else if (fmt == chars_format::fixed)
				fractional_part_end = &buffer_start[output_length];
			string_view fractional_part= string_view(fractional_part_start, size_t(fractional_part_end - fractional_part_start));
			size_t last_nonzero_digit_pos = fractional_part.find_last_not_of('0');

			char* trim_start;
			if (last_nonzero_digit_pos == string_view::npos)trim_start = decimal_point;
			else trim_start = &fractional_part_start[last_nonzero_digit_pos] + 1;
			if (fmt == chars_format::scientific)
				memmove(trim_start, fractional_part_end,size_t(&buffer_start[output_length] - fractional_part_end));
			output_length -= fractional_part_end - trim_start;
		}
		if (last-first < output_length)
			return {last, std::errc::value_too_large};

		memcpy(first, buffer_start, size_t(output_length));
		return {first + output_length,{}};
	}
	__builtin_unreachable();
}
#include <charconv>
namespace backports{
to_chars_result to_chars(char* first, char* last,       float value){return to_chars_plain(first, last, value);}
to_chars_result to_chars(char* first, char* last,      double value){return to_chars_plain(first, last, value);}
to_chars_result to_chars(char* first, char* last, long double value){return to_chars_plain(first, last, value);}
to_chars_result to_chars(char* first, char* last,       float value, chars_format fmt){return to_chars_fmt(first, last, value, fmt);}
to_chars_result to_chars(char* first, char* last,      double value, chars_format fmt){return to_chars_fmt(first, last, value, fmt);}
to_chars_result to_chars(char* first, char* last, long double value, chars_format fmt){return to_chars_fmt(first, last, value, fmt);}
to_chars_result to_chars(char* first, char* last,       float value, chars_format fmt,int precision){return to_chars_prec(first, last, value, fmt, precision);}
to_chars_result to_chars(char* first, char* last,      double value, chars_format fmt,int precision){return to_chars_prec(first, last, value, fmt, precision);}
to_chars_result to_chars(char* first, char* last, long double value, chars_format fmt,int precision){return to_chars_prec(first, last, value, fmt, precision);}
#ifdef _GLIBCXX_LONG_DOUBLE_COMPAT
// Map the -mlong-double-64 long double overloads to the double overloads.
extern "C" to_chars_result
_ZN9backports8to_charsEPcS0_d(char* first, char* last, double value,chars_format fmt)
  __attribute__((alias ("_ZN9backports8to_charsEPcS0_e")));
extern "C" to_chars_result
_ZN9backports8to_charsEPcS0_dNS_12chars_formatE(char* first, char* last, double value,chars_format fmt)
  __attribute__((alias ("_ZN9backports8to_charsEPcS0_eNS_12chars_formatE")));
extern "C" to_chars_result
_ZN9backports8to_charsEPcS0_dNS_12chars_formatEi(char* first, char* last, double value,chars_format fmt, int precision)
  __attribute__((alias ("_ZN9backports8to_charsEPcS0_eNS_12chars_formatEi")));
#endif
} // namespace backports
