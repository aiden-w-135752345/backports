#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP
#include <algorithm>
#include <functional>
#include "execution.hpp"
#include "../src/inline_variables.hpp"
namespace backports{
    namespace _algo{
        template <class T>using iterator_cat_t = typename std::iterator_traits<T>::iterator_category;
        template <class T>using iterator_diff_t = typename std::iterator_traits<T>::difference_type;
        // template <class,class>static std::false_type has_category(...);
        // template <class T,class U>static std::is_convertible<iterator_cat_t<T>, U> has_category(int);
        // template <class T>constexpr INLINE const bool is_inp = decltype(has_category<T, std::input_iterator_tag>(0))::value;
        // template <class T>constexpr INLINE const bool is_fwd = decltype(has_category<T, std::forward_iterator_tag>(0))::value;
        // template <class T>constexpr INLINE const bool is_bidi = decltype(has_category<T, std::bidirectional_iterator_tag>(0))::value;
        // template <class T>constexpr INLINE const bool is_rand = decltype(has_category<T, std::random_access_iterator_tag>(0))::value;
    }
    template<class It, class Size, class Func>
    It for_each_n(It first, Size n, Func f){
        for (Size i = 0; i < n; ++first, (void) ++i)f(*first);
        return first;
    }
    namespace _algo{
        template <class Pop,class Sample,class Count,class RNG>
        Sample sample(std::input_iterator_tag,Pop first,Pop last,std::random_access_iterator_tag,Sample dest,Count count,RNG& rng) {
            Count k = 0;
            for (; first != last && k < count; ++first, (void)++k)dest[k] = *first;
            Count sz = k;
            for (; first != last; ++first, (void)++k) {
                Count r = std::uniform_int_distribution<Count>(0, k)(rng);
                if (r < sz)dest[r] = *first;
            }
            return dest + std::min(count, k);
        }
        template <class Pop,class Sample,class Count,class RNG>
        Sample sample(std::forward_iterator_tag,Pop first,Pop last,std::input_iterator_tag,Sample dest,Count count,RNG& rng) {
            Count unsampled_sz = std::distance(first, last);
            for (count = std::min(count, unsampled_sz); count != 0; ++first) {
                Count r = std::uniform_int_distribution<Count>(0, --unsampled_sz)(rng);
                if (r < count) {*dest++ = *first;--count;}
            }
            return dest;
        }
    }
    template <class Pop, class Sample, class Count, class RNG>
    Sample sample(Pop first, Pop last, Sample dest, Count count,RNG&& rng) {
        static_assert(is_integral_v<Count>, "The sample size must have an integer type.");
        return _algo::sample(
            _algo::iterator_cat_t<Pop>(),std::move(first), std::move(last),
            _algo::iterator_cat_t<Sample>(),std::move(dest),
            std::common_type_t<Count, _algo::iterator_diff_t<Pop>>(count), rng
        );
    }
    template<class T>constexpr const T& clamp(const T& v, const T& lo, const T& hi){
        return clamp(v, lo, hi, std::less<T>{});
    }
    template<class T, class Comp>constexpr const T& clamp(const T& v, const T& lo, const T& hi, Comp comp){
        return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
    }

    /*
    template<class Policy,class It,class Pred>
    _exec::enable_if_policy<Policy,bool>all_of(Policy&&policy,It first,It last,Pred p)
    { return std::all_of(std::move(first),std::move(last),std::move(p)); }
    template<class Policy,class It,class Pred>
    _exec::enable_if_policy<Policy,bool>any_of(Policy&&policy,It first,It last,Pred p)
    { return std::any_of(std::move(first),std::move(last),std::move(p)); }
    template<class Policy,class It,class Pred>
    _exec::enable_if_policy<Policy,bool>none_of(Policy&&policy,It first,It last,Pred p)
    { return std::none_of(std::move(first),std::move(last),std::move(p)); }
    template<class Policy,class It, class Size, class Func>
    _exec::enable_if_policy<Policy,It> for_each_n(Policy&& policy,It first, Size n, Func f){
        for (Size i = 0; i < n; ++first, (void) ++i)f(*first);
        return first;
    }
    */
}// namespace backports
#endif // ALGORITHM_HPP
