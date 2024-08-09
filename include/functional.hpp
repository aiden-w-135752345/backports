#ifndef FUNCTIONAL_HPP
#define FUNCTIONAL_HPP
#include "type_traits.hpp"
#include <functional>
namespace backports{
    template<class F, class... Args> constexpr invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)noexcept(is_nothrow_invocable_v<F, Args...>);
    namespace _func{
        template<class T, class...>using first=T;
        template<class F, class... Args>using negatable=first<int,decltype(!invoke(std::declval<F>(), std::declval<Args>()...))>;
        template<class F>struct not_fn_t{
            F f;
            template<class... Args,negatable<F&, Args...> = 0> decltype(auto) operator()(Args&&... args) & {
                return !invoke(f, std::forward<Args>(args)...);
            }
            template<class... Args,negatable<const F&, Args...> = 0> decltype(auto) operator()(Args&&... args) const& {
                return !invoke(f, std::forward<Args>(args)...);
            }
            template<class... Args,negatable<F, Args...> = 0>decltype(auto) operator()(Args&&... args) && {
                return !invoke(std::move(f), std::forward<Args>(args)...);
            }
            template<class... Args,negatable<const F, Args...> = 0>decltype(auto) operator()(Args&&... args) const&& {
                return !invoke(std::move(f), std::forward<Args>(args)...);
            }
        };
    }// namespace _func
    template<class F>_func::not_fn_t<std::decay_t<F>> not_fn(F&& f){return {std::forward<F>(f)};}
    template<class F, class... Args> constexpr invoke_result_t<F, Args...> invoke(F&& f, Args&&... args)
        noexcept(is_nothrow_invocable_v<F, Args...>){
            return _invoke::invoke_traits<F, Args...>::invoke(std::forward<F>(f),std::forward<Args>(args)...);
        }
} // namespace backports
#endif // FUNCTIONAL_HPP
