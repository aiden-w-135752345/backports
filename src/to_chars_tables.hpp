#include <stdint.h>
#include "ryu/ryu_generic_128.hpp"
#include "ryu/common.hpp"
__extension__ using uint128_t = unsigned __int128;
// Determine the binary format of 'long double'.

// We support the binary64, float80 (i.e. x86 80-bit extended precision),
// binary128, and ibm128 formats.
#define LDK_UNSUPPORTED 0
#define LDK_BINARY64    1
#define LDK_FLOAT80     2
#define LDK_BINARY128   3
#define LDK_IBM128      4

#if __LDBL_MANT_DIG__ == __DBL_MANT_DIG__
	#define LONG_DOUBLE_KIND LDK_BINARY64
#elif __LDBL_MANT_DIG__ == 64
	#define LONG_DOUBLE_KIND LDK_FLOAT80
#elif __LDBL_MANT_DIG__ == 113
	#define LONG_DOUBLE_KIND LDK_BINARY128
#elif __LDBL_MANT_DIG__ == 106
	#define LONG_DOUBLE_KIND LDK_IBM128
#else
	#define LONG_DOUBLE_KIND LDK_UNSUPPORTED
#endif


	// A traits class that contains pertinent information about the binary
	// format of each of the floating-point types we support.
	template<typename T>struct floating_type_traits{};
	template<>struct floating_type_traits<float>{
		static constexpr int mantissa_bits = 23;
		static constexpr int exponent_bits = 8;
		using mantissa_t = uint32_t;
		using shortest_scientific_t = ryu::floating_decimal_32;
		static const uint64_t pow10_adjustment_tab[];
	};
	template<>struct floating_type_traits<double>{
		static constexpr int mantissa_bits = 52;
		static constexpr int exponent_bits = 11;
		using mantissa_t = uint64_t;
		using shortest_scientific_t = ryu::floating_decimal_64;

		static const uint64_t pow10_adjustment_tab[];
	};
#if LONG_DOUBLE_KIND == LDK_BINARY128 || defined FLOAT128_TO_CHARS
	// Traits for the IEEE binary128 format.
	struct floating_type_traits_binary128{
		static constexpr int mantissa_bits = 112;
		static constexpr int exponent_bits = 15;
		static constexpr bool has_implicit_leading_bit = true;
		using mantissa_t = uint128_t;
		using shortest_scientific_t = ryu::floating_decimal_128;
		static const uint64_t pow10_adjustment_tab[];
	};
#endif

#if LONG_DOUBLE_KIND == LDK_BINARY64
template<>struct floating_type_traits<long double>:template<>struct floating_type_traits<double>;
#elif LONG_DOUBLE_KIND == LDK_FLOAT80
	template<>struct floating_type_traits<long double>{
		static constexpr int mantissa_bits = 64;
		static constexpr int exponent_bits = 15;
		using mantissa_t = uint128_t;
		using shortest_scientific_t = ryu::floating_decimal_128;
		static const uint64_t pow10_adjustment_tab[];
	};
#elif LONG_DOUBLE_KIND == LDK_BINARY128
	template<>struct floating_type_traits<long double> : floating_type_traits_binary128{};
#elif LONG_DOUBLE_KIND == LDK_IBM128
	template<>struct floating_type_traits<long double>{
		static constexpr int mantissa_bits = 105;
		static constexpr int exponent_bits = 11;
		using mantissa_t = uint128_t;
		using shortest_scientific_t = ryu::floating_decimal_128;

		static const uint64_t pow10_adjustment_tab[];
	};
#endif