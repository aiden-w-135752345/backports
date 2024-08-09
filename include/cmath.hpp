#ifndef CMATH_HPP
#define CMATH_HPP
#define __STDCPP_WANT_MATH_SPEC_FUNCS__ 1
#include <cmath>
#include "type_traits.hpp"
#include <stdexcept>
namespace backports{
    namespace _cmath{
        template<class T, bool = is_integral_v<T>>struct promote{ typedef double type;};
        template<class T>struct promote<T, false>{ };
        template<>struct promote<long double>{ typedef long double type;};
        template<>struct promote<double>{ typedef double type;};
        template<>struct promote<float>{ typedef float type;};
        template<class T>using p1=typename promote<T>::type;
        template<class T, class U>using p2=decltype(p1<T>() + p1<U>());
        template<class T, class U, class V>using p3=decltype(p1<T>() + p1<U>() + p1<V>());
        template<class T>inline T hypot(T x, T y, T z){
            x=std::abs(x);y=std::abs(y);z=std::abs(z);
            T a = x<y ? (y<z?z:y) : (x<z?z:x);
            return a?a*std::sqrt((x/a)*(x/a) + (y/a)*(y/a) + (z/a)*(z/a)):a;
        }
        // +++ actually implement, or just redirect to regular cmath
        template<class T>T poly_laguerre_large_n(unsigned int n, unsigned int m, T x);
        template<class T>T poly_laguerre_recursion(unsigned int n, unsigned int m, T x);
        template<class T>T poly_laguerre_hyperg(unsigned int n, unsigned int m, T x);
        template<class T>T assoc_laguerre(unsigned int n, unsigned int m, T x){
            if (x < T(0))throw std::domain_error("Negative argument in poly_laguerre.");
            if (std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            if (n == 0)return T(1);
            if (n == 1)return T(1) + T(m) - x;
            if (x == T(0)){
                T prod = T(1);
                for (unsigned int k = 1; k <= n; ++k)prod *= (T(m) + T(k)) / T(k);
                return prod;
            }else if (n > 10000000 && x < T(2) * (T(m) + T(1)) + T(4 * n)){
                return poly_laguerre_large_n(n, m, x);
            }else if (T(m) < -T(n + 1)){
                return poly_laguerre_recursion(n, m, x);
            }else return poly_laguerre_hyperg(n, m, x);
        }
        template<class T>T assoc_legendre(unsigned int l, unsigned int m, T x,T phase = T(+1));
        template<class T>inline T beta(T x, T y){
            if (std::isnan(x) || std::isnan(y))return std::numeric_limits<T>::quiet_NaN();
            return std::exp(T(std::lgamma(x)+std::lgamma(y)-std::lgamma(x+y)));
        }
        template<class T>T ellint_rf(T x, T y, T z);
        template<class T>T ellint_rd(T x, T y, T z);
        template<class T>T ellint_rj(T x, T y, T z, T p);
        template<class T>inline T comp_ellint_1(T k){
            if (std::isnan(k)||std::abs(k)>=T(1))return std::numeric_limits<T>::quiet_NaN();
            return ellint_rf(T(0), T(1) - k * k, T(1));
        }
        template<class T>inline T comp_ellint_2(T k){
            if (std::isnan(k))return std::numeric_limits<T>::quiet_NaN();
            if (std::abs(k) == 1)return T(1);
            if (std::abs(k) > T(1))throw std::domain_error("Bad argument in comp_ellint_2.");
            const T k2 = k * k;
            return ellint_rf(T(0), T(1) - k2, T(1)) - k2*ellint_rd(T(0), T(1) - k2, T(1))/T(3);
        }
        template<class T>inline T comp_ellint_3(T k, T nu){
            if (std::isnan(k) || std::isnan(nu))return std::numeric_limits<T>::quiet_NaN();
            if (nu == T(1))return std::numeric_limits<T>::infinity();
            if (std::abs(k) > T(1))throw std::domain_error("Bad argument in comp_ellint_3.");
            const T k2 = k * k;
            return ellint_rf(T(0), T(1) - k2, T(1)) + nu*ellint_rj(T(0), T(1) - k2, T(1), T(1) - nu)/T(3);

        }
        template <class T>void bessel_ik(T nu, T x, T&Inu, T&Knu, T&Ipnu, T&Kpnu);
        template <class T>T cyl_bessel_ij_series(T nu, T x, T sgn,unsigned int max_iter);
        template <class T>void cyl_bessel_jn_asymp(T nu, T x, T&Jnu, T&Nnu);
        template <class T>void bessel_jn(T nu, T x, T&Jnu, T&Nnu, T&Jpnu, T&Npnu);
        template<class T>T cyl_bessel_i(T nu, T x){
            if (nu < T(0) || x < T(0))throw std::domain_error("Bad argument in cyl_bessel_i.");
            if (std::isnan(nu) || std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            if (x * x < T(10) * (nu + T(1)))return cyl_bessel_ij_series(nu, x, +T(1), 200);
            T I_nu, K_nu, Ip_nu, Kp_nu;
            bessel_ik(nu, x, I_nu, K_nu, Ip_nu, Kp_nu);
            return I_nu;
        }
        template<class T>T cyl_bessel_j(T nu, T x){
            if (nu < T(0) || x < T(0))throw std::domain_error("Bad argument in cyl_bessel_j.");
            if (std::isnan(nu) || std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            if (x * x < T(10) * (nu + T(1)))return cyl_bessel_ij_series(nu, x, -T(1), 200);
            if (x > T(1000)){
                T J_nu, N_nu;
                cyl_bessel_jn_asymp(nu, x, J_nu, N_nu);
                return J_nu;
            }else{
                T J_nu, N_nu, Jp_nu, Np_nu;
                bessel_jn(nu, x, J_nu, N_nu, Jp_nu, Np_nu);
                return J_nu;
            }
        }
        template<class T>T cyl_bessel_k(T nu, T x){
            if (nu < T(0) || x < T(0))throw std::domain_error("Bad argument in cyl_bessel_k.");
            if (std::isnan(nu) || std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            T I_nu, K_nu, Ip_nu, Kp_nu;
            bessel_ik(nu, x, I_nu, K_nu, Ip_nu, Kp_nu);
            return K_nu;
        }
        template<class T>T cyl_neumann(T nu, T x){
            if (nu < T(0) || x < T(0))throw std::domain_error("Bad argument in cyl_neumann.");
            if (std::isnan(nu) || std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            if (x > T(1000)){
                T J_nu, N_nu;
                cyl_bessel_jn_asymp(nu, x, J_nu, N_nu);
                return N_nu;
            }else{
                T J_nu, N_nu, Jp_nu, Np_nu;
                bessel_jn(nu, x, J_nu, N_nu, Jp_nu, Np_nu);
                return N_nu;
            }
        }
        constexpr INLINE const long double pi=3.1415926535897932384626433832795029L;
        template<class T>T ellint_1(T k, T phi){
            if (std::isnan(k) || std::isnan(phi))return std::numeric_limits<T>::quiet_NaN();
            if (std::abs(k) > T(1))throw std::domain_error("Bad argument in ellint_1.");
            const int n = std::floor(phi / static_cast<T>(pi) + T(0.5L));
            const T phi_red = phi - n*static_cast<T>(pi);
            const T s = std::sin(phi_red), c = std::cos(phi_red);
            const T F = s * ellint_rf(c * c, T(1) - k * k * s * s, T(1));
            if (n == 0) return F;
            return F + T(2) * n * comp_ellint_1(k);
        }
        template<class T>T ellint_2(T k, T phi){
            if (std::isnan(k) || std::isnan(phi))return std::numeric_limits<T>::quiet_NaN();
            if (std::abs(k) > T(1))throw std::domain_error("Bad argument in ellint_2.");
            const int n = std::floor(phi/static_cast<T>(pi) + T(0.5L));
            const T phi_red = phi - n*static_cast<T>(pi), k2 = k * k, s = std::sin(phi_red);
            const T s2 = s * s, s3 = s2 * s, c = std::cos(phi_red), c2 = c * c;
            const T E = s*ellint_rf(c2, T(1) - k2 * s2, T(1)) - k2*s3*ellint_rd(c2, T(1) - k2 * s2, T(1))/T(3);
            if (n == 0)return E;
            return E + T(2) * n * comp_ellint_2(k);
        }
        template<class T>T ellint_3(T k, T nu, T phi){
        if (std::isnan(k) || std::isnan(nu) || std::isnan(phi))return std::numeric_limits<T>::quiet_NaN();
        if (std::abs(k) > T(1))throw std::domain_error("Bad argument in ellint_3.");
            const int n = std::floor(phi/static_cast<T>(pi) + T(0.5L));
            const T phi_red = phi - n*static_cast<T>(pi), k2 = k * k, s = std::sin(phi_red);
            const T s2 = s * s, s3 = s2 * s, c = std::cos(phi_red), c2 = c * c;
            const T Pi = s*ellint_rf(c2, T(1) - k2 * s2, T(1)) + nu*s3*ellint_rj(c2, T(1) - k2 * s2, T(1), T(1) - nu * s2)/T(3);
            if (n == 0)return Pi;
            return Pi + T(2) * n * comp_ellint_3(k, nu);
        }
        template<class T>T expint_E1_series(T x);
        template<class T>T expint_En_cont_frac(unsigned int n, T x);
        template<class T>T expint_E1_asymp(T x);
        template<class T>T expint_Ei_series(T x);
        template<class T>T expint_Ei_asymp(T x);
        template<class T>inline T expint(T x){
            if(std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            if(x < T(0)){
                if (-x < T(1))
                    return -expint_E1_series(-x);
                else if (-x < T(100))
                    return -expint_En_cont_frac(1, -x);
                else return -expint_E1_asymp(-x);
            }else if (x < -std::log(std::numeric_limits<T>::epsilon())) return expint_Ei_series(x);
            else return expint_Ei_asymp(x);
        }
        template<class T>T poly_hermite_recursion(unsigned int n, T x);
        template<class T>inline T hermite(unsigned int n, T x){
            if (std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            return poly_hermite_recursion(n, x);
        }
        template<class T>inline T laguerre(unsigned int n, T x){return assoc_laguerre(n,0,x);}
        template<class T>T legendre(unsigned int l, T x);// __poly_legendre_p
        template<class T>T riemann_zeta_product(T s);
        template<class T>T riemann_zeta_glob(T s);
        template<class T>T riemann_zeta(T s){
            if (std::isnan(s))return std::numeric_limits<T>::quiet_NaN();
            if (s == T(1))return std::numeric_limits<T>::infinity();
            if (s < -T(19)){
                return riemann_zeta_product(T(1) - s)
                    * std::pow(T(2) * static_cast<T>(pi), s)
                    * std::sin(static_cast<T>(pi/2) * s)
                    * std::exp(std::lgamma(T(1) - s))
                    / static_cast<T>(pi);
            }
            else if (s < T(20)) {return riemann_zeta_glob(s);}
            else return riemann_zeta_product(s);
        }
        template<class T>void sph_bessel_jn(unsigned int n, T x, T&j_n, T&n_n, T&jp_n, T&np_n);
        template<class T>T sph_bessel(unsigned int n, T x){
            if (x < T(0))throw std::domain_error("Bad argument in sph_bessel.");
            if (std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            if (x == T(0)){
                if (n == 0)return T(1); else return T(0);
            }else{
                T j_n, n_n, jp_n, np_n;
                sph_bessel_jn(n, x, j_n, n_n, jp_n, np_n);
                return j_n;
            }
        }
        template<class T>T sph_legendre(unsigned int l, unsigned int m, T theta);
        template<class T>T sph_neumann(unsigned int n, T x){
            if (x < T(0))throw std::domain_error("Bad argument in sph_neumann.");
            if (std::isnan(x))return std::numeric_limits<T>::quiet_NaN();
            if (x == T(0))return -std::numeric_limits<T>::infinity();
            T j_n, n_n, jp_n, np_n;
            sph_bessel_jn(n, x, j_n, n_n, jp_n, np_n);
            return n_n;
        }
    }
    inline float hypot(float x,float y,float z){return _cmath::hypot<float>(x,y,z);}
    inline double hypot(double x,double y,double z){return _cmath::hypot<double>(x,y,z);}
    inline long double hypot(long double x,long double y,long double z){return _cmath::hypot<long double>(x,y,z);}
    template<class T,class U,class V>inline _cmath::p3<T,U,V>hypot(T x,U y,V z){return _cmath::hypot<_cmath::p3<T,U,V>>(x,y,z);}
    inline float assoc_laguerref(unsigned int n,unsigned int m,float x){return _cmath::assoc_laguerre<float>(n,m,x);}
    inline long double assoc_laguerrel(unsigned int n,unsigned int m,long double x){return _cmath::assoc_laguerre<long double>(n,m,x);}
    template<class T>inline _cmath::p1<T>assoc_laguerre(unsigned int n,unsigned int m,T x){return _cmath::assoc_laguerre<_cmath::p1<T>>(n,m,x);}
    inline float assoc_legendref(unsigned int l,unsigned int m,float x){return _cmath::assoc_legendre<float>(l,m,x);}
    inline long double assoc_legendrel(unsigned int l,unsigned int m,long double x){return _cmath::assoc_legendre<long double>(l,m,x);}
    template<class T>inline _cmath::p1<T>assoc_legendre(unsigned int l,unsigned int m,T x){return _cmath::assoc_legendre<_cmath::p1<T>>(l,m,x);}
    inline float betaf(float a,float b){return _cmath::beta<float>(a,b);}
    inline long double betal(long double a,long double b){return _cmath::beta<long double>(a,b);}
    template<class Ta,class Tb>inline _cmath::p2<Ta,Tb>beta(Ta a,Tb b){return _cmath::beta<_cmath::p2<Ta,Tb>>(a,b);}
    inline float comp_ellint_1f(float k){return _cmath::comp_ellint_1<float>(k);}
    inline long double comp_ellint_1l(long double k){return _cmath::comp_ellint_1<long double>(k);}
    template<class T>inline _cmath::p1<T>comp_ellint_1(T k){return _cmath::comp_ellint_1<_cmath::p1<T>>(k);}
    inline float comp_ellint_2f(float k){return _cmath::comp_ellint_2<float>(k);}
    inline long double comp_ellint_2l(long double k){return _cmath::comp_ellint_2<long double>(k);}
    template<class T>inline _cmath::p1<T>comp_ellint_2(T k){return _cmath::comp_ellint_2<_cmath::p1<T>>(k);}
    inline float comp_ellint_3f(float k,float nu){return _cmath::comp_ellint_3<float>(k,nu);}
    inline long double comp_ellint_3l(long double k,long double nu){return _cmath::comp_ellint_3<long double>(k,nu);}
    template<class T,class Tn>inline _cmath::p2<T,Tn>comp_ellint_3(T k,Tn nu){return _cmath::comp_ellint_3<_cmath::p2<T,Tn>>(k,nu);}
    inline float cyl_bessel_if(float nu,float x){return _cmath::cyl_bessel_i<float>(nu,x);}
    inline long double cyl_bessel_il(long double nu,long double x){return _cmath::cyl_bessel_i<long double>(nu,x);}
    template<class Tnu,class T>inline _cmath::p2<Tnu,T>cyl_bessel_i(Tnu nu,T x){return _cmath::cyl_bessel_i<_cmath::p2<Tnu,T>>(nu,x);}
    inline float cyl_bessel_jf(float nu,float x){return _cmath::cyl_bessel_j<float>(nu,x);}
    inline long double cyl_bessel_jl(long double nu,long double x){return _cmath::cyl_bessel_j<long double>(nu,x);}
    template<class Tnu,class T>inline _cmath::p2<Tnu,T>cyl_bessel_j(Tnu nu,T x){return _cmath::cyl_bessel_j<_cmath::p2<Tnu,T>>(nu,x);}
    inline float cyl_bessel_kf(float nu,float x){return _cmath::cyl_bessel_k<float>(nu,x);}
    inline long double cyl_bessel_kl(long double nu,long double x){return _cmath::cyl_bessel_k<long double>(nu,x);}
    template<class Tnu,class T>inline _cmath::p2<Tnu,T>cyl_bessel_k(Tnu nu,T x){return _cmath::cyl_bessel_k<_cmath::p2<Tnu,T>>(nu,x);}
    inline float cyl_neumannf(float nu,float x){return _cmath::cyl_neumann<float>(nu,x);}
    inline long double cyl_neumannl(long double nu,long double x){return _cmath::cyl_neumann<long double>(nu,x);}
    template<class Tnu,class T>inline _cmath::p2<Tnu,T>cyl_neumann(Tnu nu,T x){return _cmath::cyl_neumann<_cmath::p2<Tnu,T>>(nu,x);}
    inline float ellint_1f(float k,float phi){return _cmath::ellint_1<float>(k,phi);}
    inline long double ellint_1l(long double k,long double phi){return _cmath::ellint_1<long double>(k,phi);}
    template<class T,class Tp>inline _cmath::p2<T,Tp>ellint_1(T k,Tp phi){return _cmath::ellint_1<_cmath::p2<T,Tp>>(k,phi);}
    inline float ellint_2f(float k,float phi){return _cmath::ellint_2<float>(k,phi);}
    inline long double ellint_2l(long double k,long double phi){return _cmath::ellint_2<long double>(k,phi);}
    template<class T,class Tp>inline _cmath::p2<T,Tp>ellint_2(T k,Tp phi){return _cmath::ellint_2<_cmath::p2<T,Tp>>(k,phi);}
    inline float ellint_3f(float k,float nu,float phi){return _cmath::ellint_3<float>(k,nu,phi);}
    inline long double ellint_3l(long double k,long double nu,long double phi){return _cmath::ellint_3<long double>(k,nu,phi);}
    template<class T,class Tn,class Tp>inline _cmath::p3<T,Tn,Tp>ellint_3(T k,Tn nu,Tp phi){return _cmath::ellint_3<_cmath::p3<T,Tn,Tp>>(k,nu,phi);}
    inline float expintf(float x){return _cmath::expint<float>(x);}
    inline long double expintl(long double x){return _cmath::expint<long double>(x);}
    template<class T>inline _cmath::p1<T>expint(T x){return _cmath::expint<_cmath::p1<T>>(x);}
    inline float hermitef(unsigned int n,float x){return _cmath::hermite<float>(n,x);}
    inline long double hermitel(unsigned int n,long double x){return _cmath::hermite<long double>(n,x);}
    template<class T>inline _cmath::p1<T>hermite(unsigned int n,T x){return _cmath::hermite<_cmath::p1<T>>(n,x);}
    inline float laguerref(unsigned int n,float x){return _cmath::laguerre<float>(n,x);}
    inline long double laguerrel(unsigned int n,long double x){return _cmath::laguerre<long double>(n,x);}
    template<class T>inline _cmath::p1<T>laguerre(unsigned int n,T x){return _cmath::laguerre<_cmath::p1<T>>(n,x);}
    inline float legendref(unsigned int l,float x){return _cmath::legendre<float>(l,x);}
    inline long double legendrel(unsigned int l,long double x){return _cmath::legendre<long double>(l,x);}
    template<class T>inline _cmath::p1<T>legendre(unsigned int l,T x){return _cmath::legendre<_cmath::p1<T>>(l,x);}
    inline float riemann_zetaf(float s){return _cmath::riemann_zeta<float>(s);}
    inline long double riemann_zetal(long double s){return _cmath::riemann_zeta<long double>(s);}
    template<class T>inline _cmath::p1<T>riemann_zeta(T s){return _cmath::riemann_zeta<_cmath::p1<T>>(s);}
    inline float sph_besself(unsigned int n,float x){return _cmath::sph_bessel<float>(n,x);}
    inline long double sph_bessell(unsigned int n,long double x){return _cmath::sph_bessel<long double>(n,x);}
    template<class T>inline _cmath::p1<T>sph_bessel(unsigned int n,T x){return _cmath::sph_bessel<_cmath::p1<T>>(n,x);}
    inline float sph_legendref(unsigned int l,unsigned int m,float theta){return _cmath::sph_legendre<float>(l,m,theta);}
    inline long double sph_legendrel(unsigned int l,unsigned int m,long double theta){return _cmath::sph_legendre<long double>(l,m,theta);}
    template<class T>inline _cmath::p1<T>sph_legendre(unsigned int l,unsigned int m,T theta){return _cmath::sph_legendre<_cmath::p1<T>>(l,m,theta);}
    inline float sph_neumannf(unsigned int n,float x){return _cmath::sph_neumann<float>(n,x);}
    inline long double sph_neumannl(unsigned int n,long double x){return _cmath::sph_neumann<long double>(n,x);}
    template<class T>inline _cmath::p1<T>sph_neumann(unsigned int n,T x){return _cmath::sph_neumann<_cmath::p1<T>>(n,x);}
}// namespace backports
#endif // CMATH_HPP