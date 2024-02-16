#ifndef INVOKE_TRAITS_HPP
#define INVOKE_TRAITS_HPP 1
#include "inline_variables.hpp"
#include "../include/type_traits.hpp"
namespace backports{
    namespace _invoke{
        template<class T>struct type_identity{ using type = T; };
        template<class T>using type_identity_t = typename type_identity<T>::type;
        template<class T>using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
        template<class T,class...>using first_t = T;
        template<class>struct is_reference_wrapper:std::false_type{};
        template<class U>struct is_reference_wrapper<std::reference_wrapper<U>>:std::true_type{};
        
        template<class R,bool nt>struct test_sucess{
            using valid=std::true_type;using result=R;using nothrow=bool_constant<nt>;static R result_declval() noexcept(nt);
        };
        struct test_fail{using valid=std::false_type;using nothrow=std::false_type;};
        template<class R,bool nothrow,class F,class Arg,class...Args>struct memfun_ref_traits:test_sucess<R,nothrow>{
            static R invoke(F f,Arg&&arg,Args&&...args)noexcept(nothrow){return (std::forward<Arg>(arg).*f)(std::forward<Args>(args)...);}
        };
        template<class R,bool nothrow,class F,class Arg,class...Args>struct memfun_unwrap_traits:test_sucess<R,nothrow>{
            static R invoke(F f,Arg arg,Args&&...args)noexcept(nothrow){return (arg.get().*f)(std::forward<Args>(args)...);}
        };
        template<class R,bool nothrow,class F,class Arg,class...Args>struct memfun_deref_traits:test_sucess<R,nothrow>{
            static R invoke(F f,Arg&&arg,Args&&...args)noexcept(nothrow){return ((*std::forward<Arg>(arg)).*f)(std::forward<Args>(args)...);}
        };
        template<class F,class Arg,class...Args,class wrapped>memfun_ref_traits<
            decltype((std::declval<Arg>().*std::declval<F>())(std::declval<Args>()...)),
            noexcept((std::declval<Arg>().*std::declval<F>())(std::declval<Args>()...)),F,Arg,Args...
            >test_memptr(std::true_type,std::true_type,wrapped,int);
        template<class F,class Arg,class...Args>memfun_unwrap_traits<
            decltype(((*std::declval<Arg>()).*std::declval<F>())(std::declval<Args>()...)),
            noexcept((std::declval<Arg>().get().*std::declval<F>())(std::declval<Args>()...)),F,Arg,Args...
            >test_memptr(std::true_type,std::false_type,std::true_type,int);
        template<class F,class Arg,class...Args>memfun_deref_traits<
            decltype(((*std::declval<Arg>()).*std::declval<F>())(std::declval<Args>()...)),
            noexcept(((*std::declval<Arg>()).*std::declval<F>())(std::declval<Args>()...)),F,Arg,Args...
            >test_memptr(std::true_type,std::false_type,std::false_type,int);
        template<class R,bool nothrow,class F,class Arg>struct memobj_ref_traits:test_sucess<R,nothrow>{
            static R invoke(F f,Arg&&arg)noexcept(nothrow){return std::forward<Arg>(arg).*f;}
        };
        template<class R,bool nothrow,class F,class Arg>struct memobj_unwrap_traits:test_sucess<R,nothrow>{
            static R invoke(F f,Arg arg)noexcept(nothrow){return arg.get().*f;}
        };
        template<class R,bool nothrow,class F,class Arg>struct memobj_deref_traits:test_sucess<R,nothrow>{
            static R invoke(F f,Arg&&arg)noexcept(nothrow){return (*std::forward<Arg>(arg)).*f;}
        };
        template<class F,class Arg,class wrapped>memobj_ref_traits<
            decltype(std::declval<Arg>().*std::declval<F>()),noexcept(std::declval<Arg>().*std::declval<F>()),F,Arg
                >test_memptr(std::false_type,std::true_type,wrapped,int);
        template<class F,class Arg>memobj_unwrap_traits<
            decltype(std::declval<Arg>().*std::declval<F>()),noexcept(std::declval<Arg>().get().*std::declval<F>()),F,Arg
                >test_memptr(std::false_type,std::false_type,std::true_type,int);
        template<class F,class Arg>memobj_deref_traits<
            decltype((*std::declval<Arg>()).*std::declval<F>()),noexcept((*std::declval<Arg>()).*std::declval<F>()),F,Arg
                >test_memptr(std::false_type,std::false_type,std::false_type,int);
        template<bool,bool,bool,class,class,class...>test_fail test_memptr(...);
        
        template<class R,bool nothrow,class F,class...Args>struct functor_traits:test_sucess<R,nothrow>{
            static R invoke(F&&f,Args&&...args)noexcept(nothrow){return std::forward<F>(f)(std::forward<Args>(args)...);}
        };
        template<class F,class... Args>functor_traits<
            decltype(std::declval<F>()(std::declval<Args>()...)),noexcept(std::declval<F>()(std::declval<Args>()...)),F,Args...
            >test_functor(int);
        template<class...>test_fail test_functor(...);
        template<class,class F,class...Args>struct traits_impl:decltype(test_functor<F,Args...>(0)){};
        template<class R,class C,class F,class Arg,class...Args>struct traits_impl<R C::*,F,Arg,Args...>:decltype
            (test_memptr<R C::*,Arg,Args...>
             (
              std::is_function<R>{},
              bool_constant<is_same_v<C,remove_cvref_t<Arg>>||is_base_of_v<C,remove_cvref_t<Arg>>>{},
              is_reference_wrapper<remove_cvref_t<Arg>>{},
              0
              )){};
        template<class F,class...Args>using invoke_traits=traits_impl<std::decay_t<F>,F,Args...>;
        
        template<class T> void r_helper(type_identity_t<T>) noexcept;
        
        template<class R,bool nothrow,class Traits,class...Args>struct r_traits_impl:test_sucess<R,nothrow>{
            static R invoke(Args&&...args)noexcept(nothrow){return Traits::invoke(std::forward<Args>(args)...);}
        };
        template<class R,class Traits,class...Args>r_traits_impl<R,noexcept(r_helper<R>(Traits::result_declval())),Traits,Args...>test_r(int);
        template<class...>test_fail test_r(...);
        template<class R,class F,class...Args>using invoke_r_traits=decltype(test_r<R,invoke_traits<F,Args...>,F,Args...>(0));
    }
    template<        class F,class...Args>struct is_invocable:  _invoke::invoke_traits<F,Args...>::valid{};
    template<class R,class F,class...Args>struct is_invocable_r:_invoke::invoke_r_traits<R,F,Args...>::valid{};
    template<        class F,class...Args>struct is_nothrow_invocable:_invoke::invoke_traits<F,Args...>::nothrow{};
    template<class R,class F,class...Args>struct is_nothrow_invocable_r:_invoke::invoke_r_traits<R,F,Args...>::nothrow{};
    template<        class F,class...Args>constexpr INLINE const bool is_invocable_v = is_invocable<F,Args...>::value;
    template<class R,class F,class...Args>constexpr INLINE const bool is_invocable_r_v = is_invocable_r<R,F,Args...>::value;
    template<        class F,class...Args>constexpr INLINE const bool is_nothrow_invocable_v = is_nothrow_invocable<F,Args...>::value;
    template<class R,class F,class...Args>constexpr INLINE const bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R,F,Args...>::value;
    template<class F,class...Args>using invoke_result_t = typename _invoke::invoke_traits<F, Args...>::result;
    template<class F,class...Args>struct invoke_result {using type=invoke_result_t<F, Args...>;};
}// namespace backports
#endif //INVOKE_TRAITS_HPP