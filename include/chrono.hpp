#ifndef CHRONO_HPP
#define CHRONO_HPP
#include <chrono>
#include "../src/inline_variables.hpp"
namespace backports{namespace chrono{
    template <typename _Rep>constexpr INLINE const bool treat_as_floating_point_v
        =std::chrono::treat_as_floating_point<_Rep>::value;

    namespace _chrono{
        template<class> constexpr INLINE const bool is_duration_v = false;
        template<class Rep, class Period> constexpr INLINE const bool is_duration_v<std::chrono::duration<Rep,Period>> = true;
    }
    template<class To,class Rep,class Period,class=std::enable_if_t<_chrono::is_duration_v<To>>>
    constexpr To floor(const std::chrono::duration<Rep,Period>& d){
        To t=std::chrono::duration_cast<To>(d);
        if(t>d)return t-To{1};
        return t;
    }
    template<class To, class Rep, class Period, class=std::enable_if_t<_chrono::is_duration_v<To>>>
    constexpr To ceil(const std::chrono::duration<Rep,Period>& d){
        To t=std::chrono::duration_cast<To>(d);
        if(t<d)return t+To{1};
        return t;
    }
    template<class To, class Rep, class Period, class=std::enable_if_t<_chrono::is_duration_v<To>&&!treat_as_floating_point_v<typename To::rep>>>
    constexpr To round(const std::chrono::duration<Rep, Period>& d){
        To t0 = floor<To>(d),t1 = t0 + To{1};
        auto diff0 = d - t0;
        auto diff1 = t1 - d;
        if (diff0 == diff1){
            if (t0.count() & 1)return t1;
            return t0;
        }
        else if (diff0 < diff1)return t0;
        return t1;
    }
    template<class Rep, class Period, class=std::enable_if_t<std::numeric_limits<Rep>::is_signed>>
    constexpr std::chrono::duration<Rep, Period> abs(std::chrono::duration<Rep, Period> d){
        return d >= d.zero() ? +d : -d;
    }
    template<class To, class Clock, class From, class=std::enable_if_t<_chrono::is_duration_v<To>>>
    constexpr std::chrono::time_point<Clock, To>floor(const std::chrono::time_point<Clock, From>& tp){
        return std::chrono::time_point<Clock, To>{floor<To>(tp.time_since_epoch())};
    }
    template<class To, class Clock, class From, class=std::enable_if_t<_chrono::is_duration_v<To>>>
    constexpr std::chrono::time_point<Clock, To>ceil(const std::chrono::time_point<Clock, From>& tp){
        return std::chrono::time_point<Clock, To>{ceil<To>(tp.time_since_epoch())};
    }
    template<class To, class Clock, class From,class=std::enable_if_t<_chrono::is_duration_v<To>&&!treat_as_floating_point_v<typename To::rep>>>
    constexpr std::chrono::time_point<Clock, To> round(const std::chrono::time_point<Clock, From>& tp){
        return std::chrono::time_point<Clock, To>{round<To>(tp.time_since_epoch())};
    }
}}// namespace backports::chrono
#endif // CHRONO_HPP