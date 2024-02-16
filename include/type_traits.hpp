#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

#include "../src/inline_variables.hpp"
#include <type_traits>
#include <utility>
namespace backports{
    template< bool B >using bool_constant = std::integral_constant<bool, B>;
    template< class T >constexpr INLINE const bool is_void_v = std::is_void<T>::value;
    template< class T >constexpr INLINE const bool is_null_pointer_v = std::is_null_pointer<T>::value;
    template< class T >constexpr INLINE const bool is_integral_v = std::is_integral<T>::value;
    template< class T >constexpr INLINE const bool is_floating_point_v = std::is_floating_point<T>::value;
    template< class T >constexpr INLINE const bool is_array_v = std::is_array<T>::value;
    template< class T >constexpr INLINE const bool is_enum_v = std::is_enum<T>::value;
    template< class T >constexpr INLINE const bool is_union_v = std::is_union<T>::value;
    template< class T >constexpr INLINE const bool is_class_v = std::is_class<T>::value;
    template< class T >constexpr INLINE const bool is_function_v = std::is_function<T>::value;
    template< class T >constexpr INLINE const bool is_pointer_v = std::is_pointer<T>::value;
    template< class T >constexpr INLINE const bool is_lvalue_reference_v = std::is_lvalue_reference<T>::value;
    template< class T >constexpr INLINE const bool is_rvalue_reference_v = std::is_rvalue_reference<T>::value;
    template< class T >constexpr INLINE const bool is_member_object_pointer_v = std::is_member_object_pointer<T>::value;
    template< class T >constexpr INLINE const bool is_member_function_pointer_v = std::is_member_function_pointer<T>::value;
    template< class T >constexpr INLINE const bool is_fundamental_v = std::is_fundamental<T>::value;
    template< class T >constexpr INLINE const bool is_arithmetic_v = std::is_arithmetic<T>::value;
    template< class T >constexpr INLINE const bool is_scalar_v = std::is_scalar<T>::value;
    template< class T >constexpr INLINE const bool is_object_v = std::is_object<T>::value;
    template< class T >constexpr INLINE const bool is_compound_v = std::is_compound<T>::value;
    template< class T >constexpr INLINE const bool is_reference_v = std::is_reference<T>::value;
    template< class T >constexpr INLINE const bool is_member_pointer_v = std::is_member_pointer<T>::value;
    template< class T >constexpr INLINE const bool is_const_v = std::is_const<T>::value;
    template< class T >constexpr INLINE const bool is_volatile_v = std::is_volatile<T>::value;
    template< class T >constexpr INLINE const bool is_trivial_v = std::is_trivial<T>::value;
    template< class T >constexpr INLINE const bool is_trivially_copyable_v = std::is_trivially_copyable<T>::value;
    template< class T >constexpr INLINE const bool is_standard_layout_v = std::is_standard_layout<T>::value;
    template< class T >constexpr INLINE const bool is_pod_v = std::is_pod<T>::value;
    template< class T >constexpr INLINE const bool is_literal_type_v = std::is_literal_type<T>::value;
#ifdef _GLIBCXX_HAVE_BUILTIN_HAS_UNIQ_OBJ_REP
    template<class T>constexpr INLINE const bool has_unique_object_representations_v = __has_unique_object_representations(std::remove_cv_t<std::remove_all_extents_t<T>>);
    template<class T>struct has_unique_object_representations: bool_constant<has_unique_object_representations_v<T>>{};
#endif
    template< class T >constexpr INLINE const bool is_empty_v = std::is_empty<T>::value;
    template< class T >constexpr INLINE const bool is_polymorphic_v = std::is_polymorphic<T>::value;
    template< class T >constexpr INLINE const bool is_abstract_v = std::is_abstract<T>::value;
    template< class T >constexpr INLINE const bool is_final_v = std::is_final<T>::value;
#ifdef _GLIBCXX_HAVE_BUILTIN_IS_AGGREGATE
    template<class T>constexpr INLINE const bool is_aggregate_v = __is_aggregate(std::remove_cv_t<T>);
    template<class T>struct is_aggregate: bool_constant<is_aggregate_v<T>>{};
#endif
    template< class T >constexpr INLINE const bool is_signed_v = std::is_signed<T>::value;
    template< class T >constexpr INLINE const bool is_unsigned_v = std::is_unsigned<T>::value;
    template< class T, class... Args >constexpr INLINE const bool is_constructible_v = std::is_constructible<T, Args...>::value;
    template< class T, class... Args >constexpr INLINE const bool is_trivially_constructible_v = std::is_trivially_constructible<T, Args...>::value;
    template< class T, class... Args >constexpr INLINE const bool is_nothrow_constructible_v = std::is_nothrow_constructible<T, Args...>::value;
    template< class T >constexpr INLINE const bool is_default_constructible_v = std::is_default_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_trivially_default_constructible_v = std::is_trivially_default_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_nothrow_default_constructible_v = std::is_nothrow_default_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_copy_constructible_v = std::is_copy_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_trivially_copy_constructible_v = std::is_trivially_copy_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_nothrow_copy_constructible_v = std::is_nothrow_copy_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_move_constructible_v = std::is_move_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_trivially_move_constructible_v = std::is_trivially_move_constructible<T>::value;
    template< class T >constexpr INLINE const bool is_nothrow_move_constructible_v = std::is_nothrow_move_constructible<T>::value;
    template< class T, class U >constexpr INLINE const bool is_assignable_v = std::is_assignable<T, U>::value;
    template< class T, class U >constexpr INLINE const bool is_trivially_assignable_v = std::is_trivially_assignable<T, U>::value;
    template< class T, class U >constexpr INLINE const bool is_nothrow_assignable_v = std::is_nothrow_assignable<T, U>::value;
    template< class T >constexpr INLINE const bool is_copy_assignable_v = std::is_copy_assignable<T>::value;
    template< class T >constexpr INLINE const bool is_trivially_copy_assignable_v = std::is_trivially_copy_assignable<T>::value;
    template< class T >constexpr INLINE const bool is_nothrow_copy_assignable_v = std::is_nothrow_copy_assignable<T>::value;
    template< class T >constexpr INLINE const bool is_move_assignable_v = std::is_move_assignable<T>::value;
    template< class T >constexpr INLINE const bool is_trivially_move_assignable_v = std::is_trivially_move_assignable<T>::value;
    template< class T >constexpr INLINE const bool is_nothrow_move_assignable_v = std::is_nothrow_move_assignable<T>::value;
    template< class T >constexpr INLINE const bool is_destructible_v = std::is_destructible<T>::value;
    template< class T >constexpr INLINE const bool is_trivially_destructible_v = std::is_trivially_destructible<T>::value;
    template< class T >constexpr INLINE const bool is_nothrow_destructible_v = std::is_nothrow_destructible<T>::value;
    template< class T >constexpr INLINE const bool has_virtual_destructor_v = std::has_virtual_destructor<T>::value;
    namespace _swap {
        using std::swap;
        template<class T,class = decltype(swap(std::declval<T&>(), std::declval<T&>()))>static std::true_type swappable(int);
        template<class>static std::false_type swappable(...);
        template<class T>static bool_constant<noexcept(swap(std::declval<T&>(), std::declval<T&>()))>nothrow_swappable(int);
        template<class>static std::false_type nothrow_swappable(...);
        template<class T,class U,class = decltype(swap(std::declval<T>(), std::declval<U>())),
            class = decltype(swap(std::declval<U>(), std::declval<T>()))>static std::true_type swappable_with(int);
        template<class, class>static std::false_type swappable_with(...);
        template<class T, class U>static bool_constant<
            noexcept(swap(std::declval<T>(), std::declval<U>()))&&noexcept(swap(std::declval<U>(), std::declval<T>()))
                >nothrow_swappable_with(int);
        template<class, class>static std::false_type nothrow_swappable_with(...);
    }// namespace _swap
    template<class T>struct is_swappable: public decltype(_swap::swappable<T>(0)){};
    template<class T>struct is_nothrow_swappable: public decltype(_swap::nothrow_swappable<T>(0)){};
    template<class T, class U>struct is_swappable_with: public decltype(_swap::swappable_with<T, U>(0)){};
    template<class T, class U>struct is_nothrow_swappable_with: public decltype(_swap::nothrow_swappable_with<T, U>(0)){};
    template<class T>constexpr INLINE const bool is_swappable_v = is_swappable<T>::value;
    template<class T>constexpr INLINE const bool is_nothrow_swappable_v = is_nothrow_swappable<T>::value;
    template<class T, class U>constexpr INLINE const bool is_swappable_with_v = is_swappable_with<T, U>::value;
    template<class T, class U>constexpr INLINE const bool is_nothrow_swappable_with_v = is_nothrow_swappable_with<T, U>::value;
    template< class T >constexpr INLINE const std::size_t alignment_of_v = std::alignment_of<T>::value;
    template< class T >constexpr INLINE const std::size_t rank_v = std::rank<T>::value;
    template< class T, unsigned N = 0 >constexpr INLINE const std::size_t extent_v = std::extent<T, N>::value;
    template< class T, class U >constexpr INLINE const bool is_same_v = std::is_same<T, U>::value;
    template< class Base, class Derived >constexpr INLINE const bool is_base_of_v = std::is_base_of<Base, Derived>::value;
    template< class From, class To >constexpr INLINE const bool is_convertible_v = std::is_convertible<From, To>::value;
    template<class...>using void_t = void;
    template<class...> struct conjunction : std::true_type {};
    template<class B1> struct conjunction<B1> : B1 {};
    template<class B1, class... Bn>struct conjunction<B1, Bn...>: std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};
    template<class... B >constexpr INLINE const bool conjunction_v = conjunction<B...>::value;
    template<class...> struct disjunction : std::false_type {};
    template<class B1> struct disjunction<B1> : B1 {};
    template<class B1, class... Bn>struct disjunction<B1, Bn...>: std::conditional_t<bool(B1::value), B1, disjunction<Bn...>>  {};
    template<class...B>constexpr INLINE const bool disjunction_v = disjunction<B...>::value;
    template<class B>struct negation:bool_constant<!bool(B::value)>{};
    template<class B>constexpr INLINE const bool negation_v = negation<B>::value;
}//namespace backports
#endif //TYPE_TRAITS_HPP
#ifndef NO_INVOKE_TRAITS
#include "../src/invoke_traits.hpp"
#endif