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
#include <iostream>
#include <iomanip>
#include <cstdint>
using namespace backports;
namespace ryu{
	int to_chars(const floating_decimal_32 v, char* const result);
	int to_chars(const floating_decimal_64 v, char* const result);
	int d2fixed_buffered_n(double d, uint32_t precision, char* result) ;
	int d2exp_buffered_n(double d, uint32_t precision, char* result, int* exp_out) ;
	floating_decimal_64 floating_to_fd64(double f);
	floating_decimal_32 floating_to_fd32(float f);
	uint32_t decimalLength17(const uint64_t v);
	namespace generic128{
		floating_decimal_128 generic_binary_to_decimal(
			const uint128_t ieeeMantissa, const uint32_t ieeeExponent, const bool ieeeSign,
			const uint32_t mantissaBits, const uint32_t exponentBits, const bool explicitLeadingBit);
			uint32_t decimalLength(const uint128_t v);
	}
	int to_chars(const generic128::floating_decimal_128 v, char* const result){ return generic128::generic_to_chars(v, result); }
} // namespace ryu
template<unsigned char base,class T>static backports::to_chars_result to_chars_small(char* beg, char* end, T val){
	using U = std::conditional_t<sizeof(T)<sizeof(uint32_t),uint32_t,std::conditional_t<sizeof(T)<sizeof(uint64_t),uint64_t,uint128_t>>;
	U uval = val;
	if(beg == end) return { end, std::errc::value_too_large };
	if(val == 0){*beg = '0';return { beg + 1, std::errc{} };}
	if(val < 0){*beg++ = '-';uval=U(~val)+1;}
	U lval=uval;
	unsigned char len=1;while(lval>base){lval/=base;len++;}
	if ((end - beg) < len){return {end,std::errc::value_too_large};}
	end=beg+len;
	while(len){beg[--len]='0'+(uval%base);uval/=base;}
	return {end,{}};
}
template<class T>static backports::to_chars_result to_chars_16(char* beg, char* end, T val){
	using U = std::conditional_t<sizeof(T)<sizeof(uint32_t),uint32_t,std::conditional_t<sizeof(T)<sizeof(uint64_t),uint64_t,uint128_t>>;
	U uval = val;
	if(beg == end) return { end, std::errc::value_too_large };
	if(val == 0){*beg = '0';return { beg + 1, std::errc{} };}
	if(val < 0){*beg++ = '-';uval=U(~val)+1;}
	U lval=uval;
	unsigned char len=1;while(lval>16){lval/=16;len++;}
	if ((end - beg) < len){return {end,std::errc::value_too_large};}
	constexpr char digits[]="0123456789abcdef";
	end=beg+len;
	while(len){beg[--len]=digits[uval%16];uval/=16;}
	return {end,{}};
}
template<class T>static backports::to_chars_result to_chars_32(char* beg, char* end, T val){
	using U = std::conditional_t<sizeof(T)<sizeof(uint32_t),uint32_t,std::conditional_t<sizeof(T)<sizeof(uint64_t),uint64_t,uint128_t>>;
	U uval = val;
	if(beg == end) return { end, std::errc::value_too_large };
	if(val == 0){*beg = '0';return { beg + 1, std::errc{} };}
	if(val < 0){*beg++ = '-';uval=U(~val)+1;}
	U lval=uval;
	unsigned char len=1;while(lval>32){lval/=32;len++;}
	if ((end - beg) < len){return {end,std::errc::value_too_large};}
	constexpr char digits[]="0123456789abcdefghijklmnopqrstuv";
	end=beg+len;
	while(len){beg[--len]=digits[uval%32];uval/=32;}
	return {end,{}};
}
template<class T>static backports::to_chars_result to_chars(char* beg, char* end, T val, unsigned char base){
	switch(base){
		case 2:return to_chars_small<2>(beg,end,val);
		case 3:return to_chars_small<3>(beg,end,val);
		case 4:return to_chars_small<4>(beg,end,val);
		case 5:return to_chars_small<5>(beg,end,val);
		case 6:return to_chars_small<6>(beg,end,val);
		case 7:return to_chars_small<7>(beg,end,val);
		case 8:return to_chars_small<8>(beg,end,val);
		case 9:return to_chars_small<9>(beg,end,val);
		case 10:return to_chars_small<10>(beg,end,val);
		case 16:return to_chars_16(beg,end,val);
		case 32:return to_chars_32(beg,end,val);
		default:break;
	}
	using U = std::conditional_t<sizeof(T)<sizeof(uint32_t),uint32_t,std::conditional_t<sizeof(T)<sizeof(uint64_t),uint64_t,uint128_t>>;
	U uval = val;
	if(beg == end) return { end, std::errc::value_too_large };
	if(val == 0){*beg = '0';return { beg + 1, std::errc{} };}
	if(val < 0){*beg++ = '-';uval=U(~val)+1;}
	U lval=uval;
	unsigned char len=1;while(lval>base){lval/=base;len++;}
	if ((end - beg) < len){return {end,std::errc::value_too_large};}
	constexpr char digits[]="0123456789abcdefghijklmnopqrstuvwxyz";
	end=beg+len;
	while(len){beg[--len]=digits[uval%base];uval/=base;}
	return {end,{}};
}
namespace backports{
	to_chars_result to_chars(char*beg,char*end,         char      val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed char      val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned char      val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed short     val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned short     val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed int       val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned int       val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed long      val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned long      val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,  signed long long val,int base){return ::to_chars(beg,end,val,base);}
	to_chars_result to_chars(char*beg,char*end,unsigned long long val,int base){return ::to_chars(beg,end,val,base);}

	to_chars_result to_chars(char*beg,char*end,         char      val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,  signed char      val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned char      val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,  signed short     val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned short     val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,  signed int       val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned int       val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,  signed long      val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned long      val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,  signed long long val){return ::to_chars_small<10>(beg,end,val);}
	to_chars_result to_chars(char*beg,char*end,unsigned long long val){return ::to_chars_small<10>(beg,end,val);}
}

template<typename T>struct ieee_t{
	typename floating_type_traits<T>::mantissa_t mantissa;
	uint32_t exponent;
	bool sign;
};
// Decompose the floating-point value into its IEEE components.
template<typename T>ieee_t<T>get_ieee_repr(const T value){
	constexpr int mantissa_bits = floating_type_traits<T>::mantissa_bits;
	// frexp(T, int* exp );
	std::cout<<"value:  "<<std::hexfloat<<value<<'\n';
	ieee_t<T> ieee_repr;
	int32_t exponent;
	T mantissaFloat=std::scalbn(std::frexp(value,&exponent),mantissa_bits+1);
	ieee_repr.mantissa=mantissaFloat;
	ieee_repr.exponent=exponent;

	return ieee_repr;
}
// Invoke Ryu to obtain the shortest scientific form for the given
// floating-point number.
floating_type_traits<float>::shortest_scientific_t
floating_to_shortest_scientific(const float value){return ryu::floating_to_fd32(value);}
typename floating_type_traits<double>::shortest_scientific_t
floating_to_shortest_scientific(const double value){return ryu::floating_to_fd64(value);}
typename floating_type_traits<long double>::shortest_scientific_t
floating_to_shortest_scientific(const long double value){
	ieee_t<long double>repr=get_ieee_repr(value);
	return ryu::generic128::generic_binary_to_decimal(repr.mantissa, repr.exponent+(1u<<(floating_type_traits<long double>::exponent_bits-1))-2, repr.sign,
		floating_type_traits<long double>::mantissa_bits,
		floating_type_traits<long double>::exponent_bits,0);
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
template<typename T> bool is_rounded_up_pow10_p(const typename floating_type_traits<T>::shortest_scientific_t fd){
	if (fd.exponent < 0 || fd.mantissa != 1) return false;
	return(floating_type_traits<T>::pow10_adjustment_tab[fd.exponent/64]&(1ull<<(63-fd.exponent%64)));
}
int get_mantissa_length(const ryu::floating_decimal_32 fd){ return ryu::decimalLength9(fd.mantissa); }
int get_mantissa_length(const ryu::floating_decimal_64 fd){ return ryu::decimalLength17(fd.mantissa); }
int get_mantissa_length(const ryu::generic128::floating_decimal_128 fd){ return ryu::generic128::decimalLength(fd.mantissa); }
// This subroutine handles writing nan, inf and 0 in
// all formatting modes.
template<typename T> static optional<to_chars_result>to_chars_special(char* first, char* const last, const T value,const chars_format fmt, const int precision){
	string_view str;
	switch(std::fpclassify(value)){
	case FP_INFINITE:str = "-inf";break;
	case FP_NAN:str = "-nan";break;
	case FP_ZERO:break;
	default:case FP_SUBNORMAL:case FP_NORMAL:return nullopt;
	}
	if (!str.empty()){
		// We're formatting +-inf or +-nan.
		if (!std::signbit(value))str.remove_prefix(strlen("-"));
		if (last - first < (ptrdiff_t)str.length())return {{last, std::errc::value_too_large}};
		memcpy(first, &str[0], str.length());
		return {{first + str.length(), std::errc{}}};
	}
	// We're formatting 0.
	const bool sign = std::signbit(value);
	int expected_output_length = sign + 1;
	switch (fmt){
	case chars_format::fixed:case chars_format::scientific:case chars_format::hex:
		if (precision)expected_output_length += strlen(".") + precision;
		if (fmt == chars_format::scientific)expected_output_length += strlen("e+00");
		else if (fmt == chars_format::hex)expected_output_length += strlen("p+0");
		if (last - first < expected_output_length)return {{last, std::errc::value_too_large}};
		if (sign)*first++ = '-';
		*first++ = '0';
		if (precision){*first++ = '.';memset(first, '0', precision);first += precision;}
		if (fmt == chars_format::scientific){memcpy(first, "e+00", 4);first += 4;}
		else if (fmt == chars_format::hex){memcpy(first, "p+0", 3);first += 3;}
		break;
	case chars_format::general:default: // case chars_format{}:
		if (last - first < expected_output_length)return {{last, std::errc::value_too_large}};
		if (sign)*first++ = '-';
		*first++ = '0';
		break;
	}
	return {{first, std::errc{}}};
}
// This subroutine of the floating-point to_chars overloads performs
// hexadecimal formatting.
template<typename T>static to_chars_result to_chars_hex(char* first, char* const last, const T value,int precision){
	constexpr int mantissa_bits = floating_type_traits<T>::mantissa_bits;
	using mantissa_t = typename floating_type_traits<T>::mantissa_t;
	if (auto result = to_chars_special(first, last, value,chars_format::hex,precision<0?0:precision))
		return *result;
	// Extract the sign, mantissa and exponent from the value.
	const ieee_t<T> repr=get_ieee_repr(value);
	mantissa_t ieee_mantissa = repr.mantissa;
	const bool sign = repr.sign;

	// Calculate the unbiased exponent.
	int32_t unbiased_exponent=repr.exponent;
	// Shift the mantissa so that its bitwidth is a multiple of 4.
	constexpr unsigned rounded_mantissa_bits = (mantissa_bits + 3) / 4 * 4;
	mantissa_t effective_mantissa = ieee_mantissa << (rounded_mantissa_bits - mantissa_bits);
	// Compute the shortest precision needed to print this value exactly,
	// disregarding trailing zeros.
	constexpr int full_hex_precision = (mantissa_bits + 3) / 4;
	const int trailing_zeros = std::__countr_zero(effective_mantissa) / 4;
	const int shortest_full_precision = full_hex_precision - trailing_zeros;
	int written_exponent = unbiased_exponent;
	int effective_precision = precision<0?shortest_full_precision:precision;
	int excess_precision = 0;
	if (effective_precision < shortest_full_precision){
		// When limiting the precision, we need to determine how to round the
		// least significant printed hexit. The following branchless
		// bit-level-parallel technique computes whether to round up the
		// mantissa bit at index N (according to round-to-nearest rules) when
		// dropping N bits of precision, for each index N in the bit vector.
		// This technique is borrowed from the MSVC implementation.
		using bitvec = mantissa_t;
		const bitvec round_bit = effective_mantissa << 1;
		const bitvec has_tail_bits = round_bit - 1;
		const bitvec lsb_bit = effective_mantissa;
		const bitvec should_round = round_bit & (has_tail_bits | lsb_bit);

		const int dropped_bits = 4*(full_hex_precision - effective_precision);
		// Mask out the dropped nibbles.
		effective_mantissa >>= dropped_bits;
		effective_mantissa <<= dropped_bits;
		if (should_round & (mantissa_t{1} << dropped_bits)){
			// Round up the least significant nibble.
			effective_mantissa += mantissa_t{1} << dropped_bits;
		}
	}else{
		excess_precision = effective_precision - shortest_full_precision;
		effective_precision = shortest_full_precision;
	}
	// Compute the leading hexit and mask it out from the mantissa.
	char leading_hexit;
		const auto nibble = unsigned(effective_mantissa >> rounded_mantissa_bits);
		leading_hexit = '0' + nibble;
		effective_mantissa &= ~(mantissa_t{0b11} << rounded_mantissa_bits);
	// Now before we start writing the string, determine the total length of
	// the output string and perform a single bounds check.
	int expected_output_length = sign + 1;
	if (effective_precision + excess_precision > 0) expected_output_length += strlen(".");
	expected_output_length += effective_precision;
	const int abs_written_exponent = abs(written_exponent);
	expected_output_length += (abs_written_exponent >= 10000 ? strlen("p+ddddd")
				: abs_written_exponent >= 1000 ? strlen("p+dddd")
				: abs_written_exponent >= 100 ? strlen("p+ddd")
				: abs_written_exponent >= 10 ? strlen("p+dd")
				: strlen("p+d"));
	if (last - first < expected_output_length || last - first - expected_output_length < excess_precision)
		return {last, std::errc::value_too_large};
	char* const expected_output_end = first + expected_output_length + excess_precision;
	// Write the negative sign and the leading hexit.
	if (sign)*first++ = '-';
	*first++ = leading_hexit;

	if (effective_precision + excess_precision > 0)*first++ = '.';
	if (effective_precision > 0){
		int written_hexits = 0;
		// Extract and mask out the leading nibble after the decimal point,
		// write its corresponding hexit, and repeat until the mantissa is
		// empty.
		int nibble_offset = rounded_mantissa_bits;
		while (effective_mantissa != 0){
			nibble_offset -= 4;
			const auto nibble = unsigned(effective_mantissa >> nibble_offset);
			*first++ = "0123456789abcdef"[nibble];
			++written_hexits;
			effective_mantissa &= ~(mantissa_t{0b1111} << nibble_offset);
		}
		// Since the mantissa is now empty, every hexit hereafter must be '0'.
		if (int remaining_hexits = effective_precision - written_hexits) {
			memset(first, '0', remaining_hexits);
			first += remaining_hexits;
		}
	}

	if (excess_precision > 0){
		memset(first, '0', excess_precision);
		first += excess_precision;
	}
	// Finally, write the exponent.
	*first++ = 'p';
	if (written_exponent >= 0)*first++ = '+';
	const to_chars_result result = to_chars_small<10>(first, last, written_exponent);
	__glibcxx_assert(result.ec == std::errc{} && result.ptr == expected_output_end);
	return result;
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wabi"
template<typename T, typename... Extra> inline int sprintf_ld(char* buffer,const char* format_string, T value, Extra... args){
#if _GLIBCXX_USE_C99_FENV_TR1 && defined(FE_TONEAREST)
	const int saved_rounding_mode = fegetround();
	if (saved_rounding_mode != FE_TONEAREST)fesetround(FE_TONEAREST); // We want round-to-nearest behavior.
#endif
	int len = sprintf(buffer, format_string, args..., value);
#if _GLIBCXX_USE_C99_FENV_TR1 && defined(FE_TONEAREST)
	if (saved_rounding_mode != FE_TONEAREST)fesetround(saved_rounding_mode);
#endif
	return len;
}
#pragma GCC diagnostic pop
template<typename T>static to_chars_result to_chars_short(char* first, char* const last, const T value,chars_format fmt){
	if (fmt == chars_format::hex){return to_chars_hex(first, last, value, -1);}
	if (auto result = to_chars_special(first, last, value, fmt, 0))return *result;
	const auto fd = floating_to_shortest_scientific(value);
	const int mantissa_length = get_mantissa_length(fd);
	const int scientific_exponent = fd.exponent + mantissa_length - 1;
	if (fmt == chars_format::general){
		// Resolve the 'general' formatting mode as per the specification of
		// the 'g' printf output specifier. Since there is no precision
		// argument, the default precision of the 'g' specifier, 6, applies.
		if (scientific_exponent >= -4 && scientific_exponent < 6)fmt = chars_format::fixed;
		else fmt = chars_format::scientific;
	}else if (fmt == chars_format{}){
		// The 'plain' formatting mode resolves to 'scientific' if it yields
		// the shorter string, and resolves to 'fixed' otherwise. The
		// following lower and upper bounds on the exponent characterize when
		// to prefer 'fixed' over 'scientific'.
		int lower_bound = -(mantissa_length + 3);
		int upper_bound = 5;
		if (mantissa_length == 1)
			// The decimal point in scientific notation will be omitted in this
			// case; tighten the bounds appropriately.
			++lower_bound, --upper_bound;

		if (fd.exponent >= lower_bound && fd.exponent <= upper_bound) fmt = chars_format::fixed;
		else fmt = chars_format::scientific;
	}
	if (fmt == chars_format::scientific){
		// Calculate the total length of the output string, perform a bounds
		// check, and then defer to Ryu's to_chars subroutine.
		int expected_output_length = fd.sign + mantissa_length;
		if (mantissa_length > 1)expected_output_length += strlen(".");
		const int abs_exponent = abs(scientific_exponent);
		expected_output_length += (abs_exponent >= 1000 ? strlen("e+dddd")
					: abs_exponent >= 100 ? strlen("e+ddd")
					: strlen("e+dd"));
		if (last - first < expected_output_length)return {last, std::errc::value_too_large};
		const int output_length = ryu::to_chars(fd, first);
		return {first + output_length, std::errc{}};
	}
	else if (fmt == chars_format::fixed && fd.exponent >= 0){
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
		if (last - first < expected_output_length)return {last, std::errc::value_too_large};

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
		const int log2_mantissa = std::__bit_width(fd.mantissa) - 1;
		const bool value_fits_inside_mantissa_p
			= (log2_mantissa + (fd.exponent*10 + 2) / 3 < floating_type_traits<T>::mantissa_bits - 2);
		if (value_fits_inside_mantissa_p){
			// Print the small exactly-representable number in fixed form by
			// writing out fd.mantissa followed by fd.exponent many 0s.
			if (fd.sign)
			*first++ = '-';
			to_chars_result result = to_chars_small<10>(first, last, fd.mantissa);
			memset(result.ptr, '0', fd.exponent);
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
			char buffer[expected_output_length + 1];
			const int output_length = sprintf_ld(buffer,
							"%.0Lf", value);
			memcpy(first, buffer, output_length);
			return {first + output_length, std::errc{}};
		}else{
			// Otherwise, the number is too big, so defer to d2fixed_buffered_n.
			const int output_length = ryu::d2fixed_buffered_n(value, 0, first);
			return {first + output_length, std::errc{}};
		}
	}else if (fmt == chars_format::fixed && fd.exponent < 0){
		// The Ryu exponent is negative, so fd.mantissa definitely contains
		// all of the whole part of the number, and therefore fd.mantissa and
		// fd.exponent contain all of the information needed to format the
		// number in fixed notation "as if by std::printf" (with precision
		// equal to -fd.exponent).
		const int whole_digits = std::max<int>(mantissa_length + fd.exponent, 1);
		const int expected_output_length = fd.sign + whole_digits + strlen(".") + -fd.exponent;
		if (last - first < expected_output_length)return {last, std::errc::value_too_large};
		if (mantissa_length <= -fd.exponent){
			// The magnitude of the number is less than one.  Format the
			// number appropriately.
			if (fd.sign)*first++ = '-';
			*first++ = '0';
			*first++ = '.';
			const int leading_zeros = -fd.exponent - mantissa_length;
			memset(first, '0', leading_zeros);
			first += leading_zeros;
			return to_chars_small<10>(first, last, fd.mantissa);
		}else{
			// The magnitude of the number is at least one.
			if (fd.sign)*first++ = '-';
			to_chars_result result = to_chars_small<10>(first, last, fd.mantissa);
			// Make space for and write the decimal point in the correct spot.
			memmove(&result.ptr[fd.exponent+1], &result.ptr[fd.exponent],-fd.exponent);
			result.ptr[fd.exponent] = '.';
			return result;
		}
	}
	__builtin_unreachable();
}
template<typename T>static to_chars_result
to_chars_prec(char* first, char* const last, const T value,chars_format fmt, const int precision){
	if (fmt == chars_format::hex)return to_chars_hex(first, last, value, precision);
	if (precision < 0)
		// A negative precision argument is treated as if it were omitted, in
		// which case the default precision of 6 applies, as per the printf
		// specification.
		return to_chars_prec(first, last, value, fmt, 6);
	if (auto result = to_chars_special(first, last, value,fmt, precision))
		return *result;
	constexpr int mantissa_bits = floating_type_traits<T>::mantissa_bits;
	// Extract the sign and exponent from the value.
	const ieee_t<T> repr=get_ieee_repr(value);
	const bool sign = repr.sign;
	// Calculate the unbiased exponent.
	const int32_t unbiased_exponent = repr.exponent;
	// Obtain trunc(log2(abs(value))), which is just the unbiased exponent.
	const int floor_log2_value = unbiased_exponent;
	// This is within +-1 of log10(abs(value)). Note that log10 2 is 0.3010..
	const int approx_log10_value = (floor_log2_value >= 0
					? (floor_log2_value*301 + 999)/1000
					: (floor_log2_value*301 - 999)/1000);

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
	const int max_eff_scientific_precision
		= (floor_log2_value >= 0
		? std::max(mantissa_bits + 1, approx_log10_value + 1)
		: -(7*floor_log2_value + 9)/10 + 2 + mantissa_bits + 1);
	const int max_eff_fixed_precision
		= (floor_log2_value >= 0
		? mantissa_bits + 1
		: -floor_log2_value + mantissa_bits + 1);
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
		const int excess_precision = (fmt != chars_format::general
			? precision - effective_precision : 0);
		// Since the output of printf is locale-sensitive, we need to be able
		// to handle a radix point that's different from '.'.
		char radix[6] = {'.', '\0', '\0', '\0', '\0', '\0'};
		#ifdef RADIXCHAR
		if (effective_precision > 0)
		// ???: Can nl_langinfo() ever return null?
		if (const char* const radix_ptr = nl_langinfo(RADIXCHAR)){
			strncpy(radix, radix_ptr, sizeof(radix)-1);
			// We accept only radix points which are at most 4 bytes (one
			// UTF-8 character) wide.
			__glibcxx_assert(radix[4] == '\0');
		}
		#endif
		// Compute straightforward upper bounds on the output length.
		int output_length_upper_bound;
		if (fmt == chars_format::scientific || fmt == chars_format::general)
			output_length_upper_bound = (strlen("-d") + sizeof(radix)
						+ effective_precision
						+ strlen("e+dddd"));
		else if (fmt == chars_format::fixed){
			if (approx_log10_value >= 0)
			output_length_upper_bound = sign + approx_log10_value + 1;
			else
			output_length_upper_bound = sign + strlen("0");
			output_length_upper_bound += sizeof(radix) + effective_precision;
		}
		else __builtin_unreachable();
		// Do the sprintf into the local buffer.
		char buffer[output_length_upper_bound + 1];
		int output_length
			= sprintf_ld(buffer, output_specifier,value, effective_precision);
		if (effective_precision > 0){
			// We need to replace a radix that is different from '.' with '.'.
			const string_view radix_sv = {radix}; 
			if (radix_sv != "."){
				const string_view buffer_sv = {buffer, (size_t)output_length};
				const size_t radix_index = buffer_sv.find(radix_sv);
				if (radix_index != string_view::npos){
					buffer[radix_index] = '.';
					if (radix_sv.length() > 1){
						memmove(&buffer[radix_index + 1],
							&buffer[radix_index + radix_sv.length()],
							output_length - radix_index - radix_sv.length());
						output_length -= radix_sv.length() - 1;
					}
				}
			}
		}
		// Copy the string from the buffer over to the output range.
		if (last - first < output_length|| last - first - output_length < excess_precision)
			return {last, std::errc::value_too_large};
		memcpy(first, buffer, output_length);
		first += output_length;

		// Add the excess 0s to the result.
		if (excess_precision > 0){
			if (fmt == chars_format::scientific){
				char* const significand_end
				= (output_length >= 6 && first[-6] == 'e' ? &first[-6]
					: first[-5] == 'e' ? &first[-5]
					: &first[-4]);
				memmove(significand_end + excess_precision, significand_end,
					first - significand_end);
				memset(significand_end, '0', excess_precision);
				first += excess_precision;
			}else if (fmt == chars_format::fixed){
				memset(first, '0', excess_precision);
				first += excess_precision;
			}
		}
		return {first, std::errc{}};
	}else if (fmt == chars_format::scientific){
		const int effective_precision= std::min(precision, max_eff_scientific_precision);
		const int excess_precision = precision - effective_precision;
		// We can easily compute the output length exactly whenever the
		// scientific exponent is far enough away from +-100. But if it's
		// near +-100, then our log2 approximation is too coarse (and doesn't
		// consider precision-dependent rounding) in order to accurately
		// distinguish between a scientific exponent of +-100 and +-99.
		const bool scientific_exponent_near_100_p
			= abs(abs(floor_log2_value) - 332) <= 4;

		// Compute an upper bound on the output length. TODO: Maybe also
		// consider a lower bound on the output length.
		int output_length_upper_bound = sign + strlen("d");
		if (effective_precision > 0)output_length_upper_bound += strlen(".") + effective_precision;
		if (scientific_exponent_near_100_p|| (floor_log2_value >= 332 || floor_log2_value <= -333))
			output_length_upper_bound += strlen("e+ddd");
		else output_length_upper_bound += strlen("e+dd");
		int output_length;
		if (last - first >= output_length_upper_bound && last - first - output_length_upper_bound >= excess_precision){
			// The result will definitely fit into the output range, so we can
			// write directly into it.
			output_length = ryu::d2exp_buffered_n(value, effective_precision,
								first, nullptr);
		}else if (scientific_exponent_near_100_p){
			// Write the result of d2exp_buffered_n into an intermediate
			// buffer, do a bounds check, and copy the result into the output
			// range.
			char buffer[output_length_upper_bound];
			output_length = ryu::d2exp_buffered_n(value, effective_precision,
								buffer, nullptr);
			__glibcxx_assert(output_length == output_length_upper_bound - 1
						|| output_length == output_length_upper_bound);
			if (last - first < output_length|| last - first - output_length < excess_precision)
				return {last, std::errc::value_too_large};
			memcpy(first, buffer, output_length);
		}else
			// If the scientific exponent is not near 100, then the upper bound
			// is actually the exact length, and so the result will definitely
			// not fit into the output range.
			return {last, std::errc::value_too_large};
		first += output_length;
		if (excess_precision > 0){
			// Splice the excess zeros into the result.
			char* const significand_end = (first[-5] == 'e'? &first[-5] : &first[-4]);
			memmove(significand_end + excess_precision, significand_end,first - significand_end);
			memset(significand_end, '0', excess_precision);
			first += excess_precision;
		}
		return {first, std::errc{}};
	}else if (fmt == chars_format::fixed){
		const int effective_precision
			= std::min(precision, max_eff_fixed_precision);
		const int excess_precision = precision - effective_precision;

		// Compute an upper bound on the output length.  TODO: Maybe also
		// consider a lower bound on the output length.
		int output_length_upper_bound;
		if (approx_log10_value >= 0)output_length_upper_bound = sign + approx_log10_value + 1;
		else output_length_upper_bound = sign + strlen("0");
		if (effective_precision > 0)output_length_upper_bound += strlen(".") + effective_precision;
		int output_length;
		if (last - first >= output_length_upper_bound&& last - first - output_length_upper_bound >= excess_precision){
			// The result will definitely fit into the output range, so we can
			// write directly into it.
			output_length = ryu::d2fixed_buffered_n(value, effective_precision,
								first);
		}else{
			// Write the result of d2fixed_buffered_n into an intermediate
			// buffer, do a bounds check, and copy the result into the output
			// range.
			char buffer[output_length_upper_bound];
			output_length = ryu::d2fixed_buffered_n(value, effective_precision,
								buffer);
			__glibcxx_assert(output_length <= output_length_upper_bound);
			if (last - first < output_length
			|| last - first - output_length < excess_precision)
				return {last, std::errc::value_too_large};
			memcpy(first, buffer, output_length);
		}
		first += output_length;
		if (excess_precision > 0){
			// Append the excess zeros into the result.
			memset(first, '0', excess_precision);
			first += excess_precision;
		}
		return {first, std::errc{}};
	}else if (fmt == chars_format::general){
		// Handle the 'general' formatting mode as per C11 printf's %g output
		// specifier. Since Ryu doesn't do zero-trimming, we always write to
		// an intermediate buffer and manually perform zero-trimming there
		// before copying the result over to the output range.
		int effective_precision
			= std::min(precision, max_eff_scientific_precision + 1);
		const int output_length_upper_bound
			= strlen("-d.") + effective_precision + strlen("e+ddd");
		// The four bytes of headroom is to avoid needing to do a memmove when
		// rewriting a scientific form such as 1.00e-2 into the equivalent
		// fixed form 0.001.
		char buffer[4 + output_length_upper_bound];

		// 7.21.6.1/8: "Let P equal ... 1 if the precision is zero."
		if (effective_precision == 0)effective_precision = 1;

		// Perform a trial formatting in scientific form, and obtain the
		// scientific exponent.
		int scientific_exponent;
		char* buffer_start = buffer + 4;
		int output_length
			= ryu::d2exp_buffered_n(value, effective_precision - 1,
						buffer_start, &scientific_exponent);
		__glibcxx_assert(output_length <= output_length_upper_bound);

		// 7.21.6.1/8: "Then, if a conversion with style E would have an
		// exponent of X:
		//   if P > X >= -4, the conversion is with style f and
		//	 precision P - (X + 1).
		//   otherwise, the conversion is with style e and precision P - 1."
		const bool resolve_to_fixed_form
			= (scientific_exponent >= -4
				&& scientific_exponent < effective_precision);
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
				char* leading_digit = &buffer_start[sign];
				leading_digit[1] = leading_digit[0];
				// buffer_start == "-11234e-04"
				buffer_start -= -scientific_exponent;
				// buffer_start == "????-11234e-04"
				char* head = buffer_start;
				if (sign)*head++ = '-';
				*head++ = '0';
				*head++ = '.';
				memset(head, '0', -scientific_exponent - 1);
				// buffer_start == "-0.00011234e-04"

				// Now drop the exponent suffix, and add the leading zeros to
				// the output length.
				output_length -= strlen("e-0d");
				output_length += -scientific_exponent;
				if (effective_precision - 1 == 0)
					// The scientific form had no decimal point, but the fixed
					// form now does.
					output_length += strlen(".");
			}else if (effective_precision == 1){
				// The scientific exponent must be 0, so the fixed form
				// coincides with the scientific form (minus the exponent
				// suffix).
				output_length -= strlen("e+dd");
			}else{
				// We are dealing with a scientific form which has a
				// non-empty fractional part and a nonnegative exponent,
				// e.g. buffer_start == "1.234e+02".
				char* const decimal_point = &buffer_start[sign + 1];
				memmove(decimal_point, decimal_point+1,
					scientific_exponent);
				// buffer_start == "123.4e+02"
				decimal_point[scientific_exponent] = '.';
				if (scientific_exponent >= 100)
					output_length -= strlen("e+ddd");
				else
					output_length -= strlen("e+dd");
				if (effective_precision - 1 == scientific_exponent)
					output_length -= strlen(".");
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
			if (fmt == chars_format::scientific)decimal_point = &buffer_start[sign + 1];
			else if (fmt == chars_format::fixed)decimal_point
			= &buffer_start[output_length] - effective_precision - 1;
			char* const fractional_part_start = decimal_point + 1;
			char* fractional_part_end = nullptr;
			if (fmt == chars_format::scientific){
				fractional_part_end = (buffer_start[output_length-5] == 'e'
							? &buffer_start[output_length-5]
							: &buffer_start[output_length-4]);
			}else if (fmt == chars_format::fixed)
				fractional_part_end = &buffer_start[output_length];
			const string_view fractional_part
				= {fractional_part_start, (size_t)(fractional_part_end
								- fractional_part_start) };
			const size_t last_nonzero_digit_pos
				= fractional_part.find_last_not_of('0');

			char* trim_start;
			if (last_nonzero_digit_pos == string_view::npos)trim_start = decimal_point;
			else trim_start = &fractional_part_start[last_nonzero_digit_pos] + 1;
			if (fmt == chars_format::scientific)
				memmove(trim_start, fractional_part_end,
					&buffer_start[output_length] - fractional_part_end);
			output_length -= fractional_part_end - trim_start;
		}
		if (last - first < output_length)
			return {last, std::errc::value_too_large};

		memcpy(first, buffer_start, output_length);
		return {first + output_length, std::errc{}};
	}
	__builtin_unreachable();
}

namespace backports{
to_chars_result to_chars(char* first, char* last, float value, chars_format fmt) { return to_chars_short(first, last, value, fmt); }
to_chars_result to_chars(char* first, char* last, float value, chars_format fmt,int precision){ return to_chars_prec(first, last, value, fmt, precision); }
to_chars_result to_chars(char* first, char* last, double value, chars_format fmt){ return to_chars_short(first, last, value, fmt); }
to_chars_result to_chars(char* first, char* last, double value, chars_format fmt,int precision){ return to_chars_prec(first, last, value, fmt, precision); }
to_chars_result to_chars(char* first, char* last, long double value, chars_format fmt){return to_chars_short(first, last, value, fmt);}
to_chars_result to_chars(char* first, char* last, long double value, chars_format fmt,int precision){return to_chars_prec(first, last, value, fmt, precision);}
#ifdef _GLIBCXX_LONG_DOUBLE_COMPAT
// Map the -mlong-double-64 long double overloads to the double overloads.
extern "C" to_chars_result
_ZN9backports8to_charsEPcS0_dNS_12chars_formatE(char* first, char* last, double value,chars_format fmt)
  __attribute__((alias ("_ZN9backports8to_charsEPcS0_eNS_12chars_formatE")));

extern "C" to_chars_result
_ZN9backports8to_charsEPcS0_dNS_12chars_formatEi(char* first, char* last, double value,chars_format fmt, int precision)
  __attribute__((alias ("_ZN9backports8to_charsEPcS0_eNS_12chars_formatEi")));
#endif
} // namespace backports