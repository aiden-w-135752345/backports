#ifndef TUPLE_HPP
#define TUPLE_HPP
#include "functional.hpp"
#include "../src/inline_variables.hpp"
#include <tuple>
namespace backports{
    template<class T>constexpr INLINE const size_t tuple_size_v = std::tuple_size<T>::value;
    namespace _tuple{
        template<class F,class Tuple, size_t... I>constexpr decltype(auto) apply(F&& f, Tuple&& t, std::index_sequence<I...>){
            return invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
        }
        template<class T, class Tuple, size_t... I>constexpr T make_from(Tuple&& t, std::index_sequence<I...>){
            return T(std::get<I>(std::forward<Tuple>(t))...);
        }
        template<class Tuple>using index_seq_for=std::make_index_sequence<tuple_size_v<std::decay_t<Tuple>>>;
    }
    template< class F, class Tuple >constexpr decltype(auto) apply( F&& f, Tuple&& t ){
        return _tuple::apply(std::forward<F>(f), std::forward<Tuple>(t),_tuple::index_seq_for<Tuple>{});
    }
    template< class T, class Tuple > constexpr T make_from_tuple( Tuple&& t ){
        return _tuple::make_from<T>(std::forward<Tuple>(t),_tuple::index_seq_for<Tuple>{});
    }
}//namespace backports
#endif //TUPLE_HPP