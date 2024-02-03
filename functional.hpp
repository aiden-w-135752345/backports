#include "type_traits.hpp"
#include <functional>
#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP

#ifndef CONST_INLINE
#define CONST_INLINE const static
#endif
namespace backports{
    template<class F, class... Args> constexpr invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)noexcept(is_nothrow_invocable_v<F, Args...>);
    namespace detail{
        template<class V, class F, class... Args>constexpr bool negate_invocable_impl = false;
        template<class F, class... Args>constexpr bool negate_invocable_impl<
            void_t<decltype(!invoke(std::declval<F>(), std::declval<Args>()...))>, F, Args...> = true;
        template<class F, class... Args>constexpr bool negate_invocable_v = negate_invocable_impl<void, F, Args...>;
        template<class F>struct not_fn_t{
            F f;
            template<class... Args,std::enable_if_t<negate_invocable_v<F&, Args...>, int> = 0>
                decltype(auto) operator()(Args&&... args) & { return !invoke(f, std::forward<Args>(args)...);}
            template<class... Args,std::enable_if_t<negate_invocable_v<const F&, Args...>, int> = 0>
                decltype(auto) operator()(Args&&... args) const& { return !invoke(f, std::forward<Args>(args)...); }
            template<class... Args,std::enable_if_t<negate_invocable_v<F, Args...>, int> = 0>
                decltype(auto) operator()(Args&&... args) && { return !invoke(std::move(f), std::forward<Args>(args)...); }
            template<class... Args,std::enable_if_t<negate_invocable_v<const F, Args...>, int> = 0>
                decltype(auto) operator()(Args&&... args) const&& { return !invoke(std::move(f), std::forward<Args>(args)...); }
        };
    }// namespace detail
    template<class F>detail::not_fn_t<std::decay_t<F>> not_fn(F&& f){return {std::forward<F>(f)};}
    template<class F, class... Args> constexpr invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
        noexcept(is_nothrow_invocable_v<F, Args...>){
            return detail::invoke_traits<F, Args...>::invoke(std::forward<F>(f),std::forward<Args>(args)...);
        }
} // namespace backports
#endif // FUNCTIONAL_HPP