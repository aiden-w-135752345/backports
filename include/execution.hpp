#ifndef EXECUTION_HPP
#define EXECUTION_HPP
#include <execution>
#include "../src/inline_variables.hpp"
#include "type_traits.hpp"
namespace backports{
    namespace execution {
        class sequenced_policy{};
        class parallel_policy{};
        class parallel_unsequenced_policy{};
        class unsequenced_policy{};
        INLINE constexpr sequenced_policy            seq{};
        INLINE constexpr parallel_policy             par{};
        INLINE constexpr parallel_unsequenced_policy par_unseq{};
        INLINE constexpr unsequenced_policy          unseq{};
    }
    template<class T>struct is_execution_policy:std::false_type{};
    template<>struct is_execution_policy<execution::sequenced_policy>:std::true_type{};
    template<>struct is_execution_policy<execution::parallel_policy>:std::true_type{};
    template<>struct is_execution_policy<execution::parallel_unsequenced_policy>:std::true_type{};
    template<>struct is_execution_policy<execution::unsequenced_policy>:std::true_type{};
    template<class T>constexpr INLINE const bool is_execution_policy_v = is_execution_policy<T>::value;
    namespace _exec{
        template<class Policy,class Value=void>using enable_if_policy=std::enable_if_t<is_execution_policy_v<std::decay_t<Policy>>,Value>;
    }
}// namespace backports
#endif // EXECUTION_HPP
