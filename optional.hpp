#include "utility.hpp"
#include "type_traits.hpp"
#include <stdexcept>
#include <new>
#include <initializer_list>
#include <functional>

#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP 1
#ifndef CONST_INLINE
#define CONST_INLINE const static
#endif
namespace backports{
    struct nullopt_t{};
    constexpr CONST_INLINE nullopt_t nullopt;
    struct bad_optional_access : public std::exception{
        bad_optional_access() {}
        const char* what() const noexcept override{return "bad optional access";}
    };
    namespace detail{
        template <class T,
            bool cc = is_copy_constructible_v<T>,
            bool tcc= is_trivially_copy_constructible_v<T>,
            bool mc = is_move_constructible_v<T>,
            bool tmc= is_trivially_move_constructible_v<T>,
            bool ca = is_copy_constructible_v<T> && is_copy_assignable_v<T>,
            bool tca= is_trivially_copy_constructible_v<T> && is_trivially_copy_assignable_v<T>,
            bool ma = is_move_constructible_v<T> && is_move_assignable_v<T>,
            bool tma= is_trivially_move_constructible_v<T> && is_trivially_move_assignable_v<T>,
            bool td = is_trivially_destructible_v<T>
            >struct Base;
#define TEMPLATE(CC,TCC,MC,TMC,CA,TCA,MA,TMA,TD,COPYCTOR,MOVECTOR,COPYASSIGN,MOVEASSIGN,DTOR) \
    template <class T>struct Base<T,CC,TCC,MC,TMC,CA,TCA,MA,TMA,TD> { \
        using Payload = std::remove_const_t<T>; \
        bool filled = false; \
        union { nullopt_t nothing; Payload payload; }; \
        constexpr Base()=default; \
        template<class... Args,std::enable_if_t<is_constructible_v<T,Args&&...>,bool> = false>constexpr Base(in_place_t, Args&&... args) \
            :filled(true),payload(std::forward<Args>(args)...){} \
        template<class... Args>void construct(in_place_t,Args&&... args)noexcept(is_nothrow_constructible_v<Payload, Args...>){ \
            ::new ((void *) std::addressof(this->payload))Payload(std::forward<Args>(args)...); \
            this->filled = true; \
        } \
        template<class... Args>void construct(const Base&that)noexcept(is_nothrow_copy_constructible_v<Payload>){ \
            if(that.filled){::new ((void *) std::addressof(this->payload))Payload(that.payload); this->filled = true; } \
        } \
        template<class... Args>void construct(const Base&&that)noexcept(is_nothrow_copy_constructible_v<Payload>){ \
            if(that.filled){::new ((void *) std::addressof(this->payload))Payload(std::move(that.payload)); this->filled = true; } \
        } \
        void assign(const Base&that)noexcept(is_nothrow_copy_constructible_v<Payload>&&is_nothrow_copy_assignable_v<Payload>){ \
            if (this->filled && that.filled) this->payload = that.payload; \
            else {this->reset();this->construct(that); } \
        } \
        void assign(Base&&that)noexcept(is_nothrow_move_constructible_v<Payload>&&is_nothrow_move_assignable_v<Payload>){ \
            if (this->filled && that.filled) this->payload = std::move(that.payload); \
            else {this->reset();this->construct(std::move(that)); } \
        } \
        void reset(){if(filled){payload.~Payload();filled=false;}} \
        constexpr bool has_value() const noexcept { return this->filled; } \
        constexpr T&get() noexcept { return this->payload; } \
        constexpr const T& get() const noexcept { return this->payload; } \
        void assert_filled() const{if(!this->filled){throw bad_optional_access();}} \
        constexpr Base(const Base& that) COPYCTOR \
            constexpr Base(Base&& that) MOVECTOR \
            constexpr Base& operator=(const Base& that) COPYASSIGN \
            constexpr Base& operator=(Base&& that) MOVEASSIGN \
            ~Base() DTOR \
        }
#include "detail/constexpr_container.hpp"
#undef TEMPLATE
    }// namespace detail
    template<class T>class optional: private detail::Base<T>{
        static_assert(!is_same_v<std::remove_cv_t<T>, nullopt_t>&&
                      !is_same_v<std::remove_cv_t<T>, in_place_t>&&
                      !is_reference_v<T>,"Invalid instantiation of optional<T>");
        using Base = detail::Base<T>;
        template<class U>constexpr CONST_INLINE bool not_optional_T = !is_same_v<optional<T>,std::decay_t<U>>;
        template<class U>constexpr CONST_INLINE bool not_in_place = !is_same_v<in_place_t,std::decay_t<U>>;
        template<class...Args>constexpr CONST_INLINE bool T_constructible=is_constructible_v<T,Args...>;
        template<class U>constexpr CONST_INLINE bool convertible_T=is_convertible_v<U,T>;
        template<class U>constexpr CONST_INLINE bool not_convertible_from_optional =
            !(is_same_v<T, U>||
              T_constructible<const optional<U>&>||T_constructible<optional<U>&>||
              T_constructible<const optional<U>&&>||T_constructible<optional<U>&&>||
              convertible_T<const optional<U>&>||convertible_T<optional<U>&>||
              convertible_T<const optional<U>&&>||convertible_T<optional<U>&&>
              );
        template<class U>constexpr CONST_INLINE bool not_assignable_from_optional=
            !(is_assignable_v<T&,const optional<U>&>||is_assignable_v<T&, optional<U>&>||
              is_assignable_v<T&,const optional<U>&&>||is_assignable_v<T&, optional<U>&&>
              )&&is_assignable_v<T&, U>&&not_convertible_from_optional<U>;
    public:
        using value_type = T;
        constexpr optional() = default;
        constexpr optional(nullopt_t) noexcept: Base() {}
        // Converting constructors for engaged optionals.
        template <class U = T,
            std::enable_if_t<not_optional_T<U>&&not_in_place<U>&&T_constructible<U&&>&&convertible_T<U&&>, bool> = true>
            constexpr optional(U&& u) : Base(in_place, std::forward<U>(u)) { }
        template <class U = T,
            std::enable_if_t<not_optional_T<U>&&not_in_place<U>&&T_constructible<U&&>&&!convertible_T<U&&>, bool> = false>
            explicit constexpr optional(U&& u): Base(in_place, std::forward<U>(u)) { }
        template <class U,
            std::enable_if_t<T_constructible<const U&>&&not_convertible_from_optional<U>&&convertible_T<const U&>, bool> = true>
            constexpr optional(const optional<U>& u){if (u)emplace(*u);}
        template <class U,
            std::enable_if_t<T_constructible<const U&>&&not_convertible_from_optional<U>&&!convertible_T<const U&>, bool> = false>
            explicit constexpr optional(const optional<U>& u){if (u)emplace(*u);}
        template <class U,
            std::enable_if_t<T_constructible<U&&>&&not_convertible_from_optional<U>&&convertible_T<U&&>, bool> = true>
            constexpr optional(optional<U>&&u){if (u)emplace(std::move(*u));}
        // in_place
        template <class U,
            std::enable_if_t<T_constructible<U&&>&&not_convertible_from_optional<U>&&!convertible_T<U&&>, bool> = false>
            explicit constexpr optional(optional<U>&& u){if (u)emplace(std::move(*u));}
        template<class... Args,std::enable_if_t<T_constructible<Args&&...>, bool> = false>
            explicit constexpr optional(in_place_t, Args&&...args)
                :Base(in_place, std::forward<Args>(args)...){}
        template<class U, class... Args,std::enable_if_t<T_constructible<std::initializer_list<U>&, Args&&...>, bool> = false>
            explicit constexpr optional(in_place_t, std::initializer_list<U>il, Args&&...args)
                :Base(in_place, il, std::forward<Args>(args)...){}
        // Assignment operators.
        optional&operator=(nullopt_t) noexcept {this->reset();return *this;}
        template<class U = T>std::enable_if_t<
            not_optional_T<U>&&T_constructible<U>&&is_assignable_v<T&, U>&&
            !(is_scalar_v<T>&&is_same_v<T, std::decay_t<U>>),
        optional&>operator=(U&& u){
            if (this->filled)this->payload = std::forward<U>(u);
            else this->construct(in_place,std::forward<U>(u));
            return *this;
        }
        template<class U>std::enable_if_t<T_constructible<const U&>&&not_assignable_from_optional<U>,optional&>
            operator=(const optional<U>& u){
                if (u){
                    if (this->filled)this->payload = *u;
                    else this->construct(*u);
                }
                else this->reset();
                return *this;
            }
        
        template<class U>std::enable_if_t<T_constructible<U&&>&&not_assignable_from_optional<U>,optional&>
            operator=(optional<U>&& u){
                if (u){
                    if (this->filled)this->payload = std::move(*u);
                    else this->construct(in_place,std::move(*u));
                }
                else this->reset();
                
                return *this;
            }
        
        template<class... Args>std::enable_if_t<T_constructible<Args&&...>, T&>emplace(Args&&... args){
            this->reset();
            this->construct(in_place,std::forward<Args>(args)...);
            return this->payload;
        }
        
        template<class U, class... Args>std::enable_if_t<T_constructible<std::initializer_list<U>&,Args&&...>, T&>
            emplace(std::initializer_list<U> il, Args&&... args){
                this->reset();
                this->construct(in_place, il, std::forward<Args>(args)...);
                return this->payload;
            }
        // Swap.
        void swap(optional& that) noexcept(is_nothrow_move_constructible_v<T> && is_nothrow_swappable_v<T>){
            using std::swap;
            if (this->has_value() && that.has_value()) swap(this->payload, that.payload);
            else if (this->has_value()){
                that.construct(std::move(this->payload));
                this->reset();
            }
            else if (that.has_value()){
                this->construct(std::move(that.payload));
                that.reset();
            }
        }
        
        // Observers.
        constexpr const T*operator->() const{ return std::addressof(this->payload); }
        T*operator->(){ return std::addressof(this->payload); }
        constexpr const T&operator*() const&{ return this->payload; }
        constexpr T&operator*()&{ return this->payload; }
        constexpr T&&operator*()&&{ return std::move(this->payload); }
        constexpr const T&&operator*() const&&{ return std::move(this->payload); }
        using Base::has_value;
        constexpr explicit operator bool() const noexcept{ return this->filled; }
        constexpr const T&value() const& {this->assert_filled();return this->payload;}
        constexpr T&value()&{this->assert_filled();return this->payload;}
        constexpr T&&value()&&{this->assert_filled();return std::move(this->payload);}
        constexpr const T&&value() const&&{this->assert_filled();return std::move(this->payload);}
        template<class U>constexpr T value_or(U&& u) const&{
            static_assert(is_copy_constructible_v<T>&&convertible_T<U&&>,"Cannot return value");
            return this->filled? this->payload: static_cast<T>(std::forward<U>(u));
        }
        template<class U>T value_or(U&& u) && {
            static_assert(is_move_constructible_v<T>&&convertible_T<U&&>,"Cannot return value" );
            return this->filled? std::move(this->payload): static_cast<T>(std::forward<U>(u));
        }
        using Base::reset;
        // Destructor is implicit, implemented in Base.
    };// class optional
    namespace detail{
        template<class T>using relop_t = std::enable_if_t<is_convertible_v<T, bool>, bool>;
        template<class T,class U>using eq_t=relop_t<decltype(std::declval<T>() == std::declval<U>())>;
        template<class T,class U>using ne_t=relop_t<decltype(std::declval<T>() != std::declval<U>())>;
        template<class T,class U>using lt_t=relop_t<decltype(std::declval<T>() <  std::declval<U>())>;
        template<class T,class U>using le_t=relop_t<decltype(std::declval<T>() <= std::declval<U>())>;
        template<class T,class U>using ge_t=relop_t<decltype(std::declval<T>() >= std::declval<U>())>;
        template<class T,class U>using gt_t=relop_t<decltype(std::declval<T>() >  std::declval<U>())>;
    }
    // Comparisons between optional values.
    template<class T, class U>constexpr detail::eq_t<T,U> operator==(const optional<T>& lhs, const optional<U>& rhs)
    {return lhs.has_value() == rhs.has_value() && (!lhs || *lhs == *rhs);}
    template<class T, class U>constexpr detail::ne_t<T,U> operator!=(const optional<T>& lhs, const optional<U>& rhs)
    { return lhs.has_value() != rhs.has_value() || (lhs.has_value() && *lhs != *rhs);}
    
    template<class T, class U> constexpr detail::lt_t<T,U> operator<(const optional<T>& lhs, const optional<U>& rhs)
    { return rhs.has_value() && (!lhs || *lhs < *rhs); }
    template<class T, class U> constexpr detail::le_t<T,U> operator<=(const optional<T>& lhs, const optional<U>& rhs)
    { return !lhs || (rhs.has_value() && *lhs <= *rhs); }
    template<class T, class U> constexpr detail::ge_t<T,U> operator>=(const optional<T>& lhs, const optional<U>& rhs)
    { return !rhs || (lhs.has_value() && *lhs >= *rhs); }
    template<class T, class U> constexpr detail::gt_t<T,U> operator>(const optional<T>& lhs, const optional<U>& rhs)
    { return lhs.has_value() && (!rhs || *lhs > *rhs); }
    // Comparisons with nullopt.
    template<class T> constexpr bool operator==(const optional<T>& lhs, nullopt_t) noexcept{ return !lhs; }
    template<class T> constexpr bool operator==(nullopt_t, const optional<T>& rhs) noexcept{ return !rhs; }
    template<class T> constexpr bool operator!=(const optional<T>& lhs, nullopt_t) noexcept{ return lhs.has_value(); }
    template<class T>constexpr bool operator!=(nullopt_t, const optional<T>& rhs) noexcept{ return rhs.has_value(); }
    template<class T>constexpr bool operator<(const optional<T>& /* lhs */, nullopt_t) noexcept{ return false; }
    template<class T>constexpr bool operator<(nullopt_t, const optional<T>& rhs) noexcept{ return rhs.has_value(); }
    template<class T>constexpr bool operator>(const optional<T>& lhs, nullopt_t) noexcept{ return lhs.has_value(); }
    template<class T>constexpr bool operator>(nullopt_t, const optional<T>& /* rhs */) noexcept{ return false; }
    template<class T>constexpr bool operator<=(const optional<T>& lhs, nullopt_t) noexcept{ return !lhs; }
    template<class T>constexpr bool operator<=(nullopt_t, const optional<T>& /* rhs */) noexcept{ return true; }
    template<class T>constexpr bool operator>=(const optional<T>& /* lhs */, nullopt_t) noexcept{ return true; }
    template<class T>constexpr bool operator>=(nullopt_t, const optional<T>& rhs) noexcept{ return !rhs; }
    // Comparisons with value type.
    template<class T, class U> constexpr detail::eq_t<T,U>
        operator==(const optional<T>& lhs, const U& rhs){ return lhs && *lhs == rhs; }
    template<class T, class U> constexpr detail::eq_t<T,U>
        operator==(const T& lhs, const optional<U>& rhs){ return rhs && lhs == *rhs; }
    template<class T, class U> constexpr detail::ne_t<T,U>
        operator!=(const optional<T>& lhs, const U& rhs){ return !lhs || *lhs != rhs; }
    template<class T, class U> constexpr detail::ne_t<T,U>
        operator!=(const T& lhs, const optional<U>& rhs){ return !rhs || lhs != *rhs; }
    template<class T, class U> constexpr detail::lt_t<T,U>
        operator<(const optional<T>& lhs, const U& rhs){ return !lhs || *lhs < rhs; }
    template<class T, class U> constexpr detail::lt_t<T,U>
        operator<(const T& lhs, const optional<U>& rhs){ return rhs && lhs < *rhs; }
    template<class T, class U> constexpr detail::le_t<T,U>
        operator<=(const optional<T>& lhs, const U& rhs){ return !lhs || *lhs <= rhs; }
    template<class T, class U> constexpr detail::le_t<T,U>
        operator<=(const T& lhs, const optional<U>& rhs){ return rhs && lhs <= *rhs; }
    template<class T, class U> constexpr detail::ge_t<T,U>
        operator>=(const optional<T>& lhs, const U& rhs){ return lhs && *lhs >= rhs; }
    template<class T, class U> constexpr detail::ge_t<T,U>
        operator>=(const T& lhs, const optional<U>& rhs){ return !rhs || lhs >= *rhs; }
    template<class T, class U> constexpr detail::gt_t<T,U>
        operator>(const optional<T>& lhs, const U& rhs){ return lhs && *lhs > rhs; }
    template<class T, class U> constexpr detail::gt_t<T,U>
        operator>(const T& lhs, const optional<U>& rhs){ return !rhs || lhs > *rhs; }
    // Swap and creation functions.
    template<class T>inline std::enable_if_t<is_move_constructible_v<T>&& is_swappable_v<T>>
        swap(optional<T>& lhs, optional<T>& rhs)noexcept(noexcept(lhs.swap(rhs))){ lhs.swap(rhs); }
    template<class T>std::enable_if_t<!(is_move_constructible_v<T>&& is_swappable_v<T>)>
        swap(optional<T>&, optional<T>&) = delete;
    template<class T>constexpr optional<std::decay_t<T>>make_optional(T&& t)
    { return optional<std::decay_t<T>> { std::forward<T>(t) }; }
    template<class T, class ...Args>constexpr optional<T>make_optional(Args&&... args)
    { return optional<T> { in_place, std::forward<Args>(args)... }; }
    template<class T, class U, class ...Args>constexpr optional<T>make_optional(std::initializer_list<U> il, Args&&... args)
    { return optional<T> { in_place, il, std::forward<Args>(args)... }; }
    // Hash.
    template<class T, class U = std::remove_const_t<T>,bool = std::__poison_hash<U>::__enable_hash_call>
        struct optional_hash{};
    template<class T, class U>struct optional_hash<T, U, true> {
        size_t operator()(const optional<T>& t) const noexcept(noexcept(std::hash<U>{}(*t))){
            return t ? std::hash<U>{}(*t) : static_cast<size_t>(-3333);
        }
    };
} // namespace opt

template<class T>struct std::hash<backports::optional<T>>
    :private __poison_hash<std::remove_const_t<T>>,public backports::optional_hash<T>{
        using result_type = size_t;using argument_type = backports::optional<T>;
    };
#if __cpp_deduction_guides >= 201606
template <class T> optional(T) -> optional<T>;
#endif

#endif // OPTIONAL_HPP