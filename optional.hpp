#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP 1
#include "utility.hpp"
#include "type_traits.hpp"
#include "inline_variables.hpp"
#include "detail/special_members.hpp"
#include <stdexcept>
#include <new>
#include <initializer_list>
#include <functional>
namespace backports{
    struct nullopt_t{};
    constexpr INLINE const nullopt_t nullopt;
    struct bad_optional_access : public std::exception{
        bad_optional_access() {}
        const char* what() const noexcept override{return "bad optional access";}
    };
    namespace _opt{
        template <class T>struct Base{
            using Payload = std::remove_const_t<T>;
            bool filled = false;
            alignas(T) unsigned char payload[sizeof(T)];
            constexpr Base()=default;
            void construct(const Base&that)noexcept(is_nothrow_copy_constructible_v<Payload>){
                if(that.filled){::new (this->payload)Payload(*(T*)that.payload); this->filled = true; }
            }
            void construct(const Base&&that)noexcept(is_nothrow_move_constructible_v<Payload>){
                if(that.filled){::new (this->payload)Payload(std::move(*(T*)that.payload)); this->filled = true; }
            }
            void assign(const Base&that)noexcept(is_nothrow_copy_constructible_v<Payload>&&is_nothrow_copy_assignable_v<Payload>){
                if (this->filled && that.filled) *(T*)(this->payload) = *(T*)that.payload;
                else {this->reset();this->construct(that); }
            }
            void assign(Base&&that)noexcept(is_nothrow_move_constructible_v<Payload>&&is_nothrow_move_assignable_v<Payload>){
                if (this->filled && that.filled) *(T*)(this->payload) = std::move(*(T*)that.payload);
                else {this->reset();this->construct(std::move(that)); }
            }
            void destruct(){reset();}
            void reset(){if(filled){((T*)payload)->~Payload();filled=false;}}
        };
        template <class T>using Base_t=detail::special_members<
            Base<T>,
            is_copy_constructible_v<T>,
            is_trivially_copy_constructible_v<T>,
            is_move_constructible_v<T>,
            is_trivially_move_constructible_v<T>,
            is_copy_constructible_v<T> && is_copy_assignable_v<T>,
            is_trivially_copy_constructible_v<T> && is_trivially_copy_assignable_v<T>,
            is_move_constructible_v<T> && is_move_assignable_v<T>,
            is_trivially_move_constructible_v<T> && is_trivially_move_assignable_v<T>,
            is_trivially_destructible_v<T>
            >;
    }// namespace _opt
    template<class T>class optional: private _opt::Base_t<T>{
        static_assert(!is_same_v<std::remove_cv_t<T>, nullopt_t>&&
                      !is_same_v<std::remove_cv_t<T>, in_place_t>&&
                      !is_reference_v<T>,"Invalid instantiation of optional<T>");
        using Base = _opt::Base_t<T>;
        template<class U>constexpr INLINE const bool not_optional_T = !is_same_v<optional<T>,std::decay_t<U>>;
        template<class U>constexpr INLINE const bool not_in_place = !is_same_v<in_place_t,std::decay_t<U>>;
        template<class...Args>constexpr INLINE const bool T_constructible=is_constructible_v<T,Args...>;
        template<class...Args>constexpr INLINE const bool T_nothrow_constructible=is_nothrow_constructible_v<T, Args...>;
        template<class U>constexpr INLINE const bool convertible_T=is_convertible_v<U,T>;
        template<class U>constexpr INLINE const bool not_convertible_from_optional =
            !(is_same_v<T, U>||
              T_constructible<const optional<U>&>||T_constructible<optional<U>&>||
              T_constructible<const optional<U>&&>||T_constructible<optional<U>&&>||
              convertible_T<const optional<U>&>||convertible_T<optional<U>&>||
              convertible_T<const optional<U>&&>||convertible_T<optional<U>&&>
              );
        template<class U>constexpr INLINE const bool not_assignable_from_optional=
            !(is_assignable_v<T&,const optional<U>&>||is_assignable_v<T&, optional<U>&>||
              is_assignable_v<T&,const optional<U>&&>||is_assignable_v<T&, optional<U>&&>
              )&&is_assignable_v<T&, U>&&not_convertible_from_optional<U>;
        template<class U>using InitList=std::initializer_list<U>;
        using Base::filled;using Base::payload;
        void assert_filled() const{if(!filled){throw bad_optional_access();}}
        template<class... Args>T&emplace_impl(Args&&... args){
            reset();
            using Payload = std::remove_const_t<T>;
            ::new (payload)Payload(std::forward<Args>(args)...);
            filled = true;
            return *(T*)payload;
        }
    public:
        using value_type = T;
        constexpr optional() = default;
        constexpr optional(nullopt_t) noexcept: Base() {}
        // Special members are implicit, implemented in Base.
        // Converting constructors for engaged optionals.
        template <class U = T,
            std::enable_if_t<not_optional_T<U>&&not_in_place<U>&&T_constructible<U&&>&&convertible_T<U&&>, bool> = true>
            constexpr optional(U&& u){emplace(std::forward<U>(u));}
        template <class U = T,
            std::enable_if_t<not_optional_T<U>&&not_in_place<U>&&T_constructible<U&&>&&!convertible_T<U&&>, bool> = false>
            explicit constexpr optional(U&& u){emplace(std::forward<U>(u));}
        template <class U,
            std::enable_if_t<T_constructible<const U&>&&not_convertible_from_optional<U>&&convertible_T<const U&>, bool> = true>
            constexpr optional(const optional<U>&that){if(that.filled)emplace(*(U*)that.payload);}
        template <class U,
            std::enable_if_t<T_constructible<const U&>&&not_convertible_from_optional<U>&&!convertible_T<const U&>, bool> = false>
            explicit constexpr optional(const optional<U>&that){if(that.filled)emplace(*(U*)that.payload);}
        template <class U,
            std::enable_if_t<T_constructible<U&&>&&not_convertible_from_optional<U>&&convertible_T<U&&>, bool> = true>
            constexpr optional(optional<U>&&that){if(that.filled)emplace(std::move(*(U*)that.payload));}
        template <class U,
            std::enable_if_t<T_constructible<U&&>&&not_convertible_from_optional<U>&&!convertible_T<U&&>, bool> = false>
            explicit constexpr optional(optional<U>&& that){if(that.filled)emplace(std::move(*(U*)that.payload));}
        
        template<class... Args,std::enable_if_t<T_constructible<Args&&...>, bool> = false>
            explicit constexpr optional(in_place_t, Args&&...args){emplace(std::forward<Args>(args)...);}
        template<class U, class... Args,std::enable_if_t<T_constructible<InitList<U>&, Args&&...>, bool> = false>
            explicit constexpr optional(in_place_t, InitList<U>il, Args&&...args){emplace(il,std::forward<Args>(args)...);}
        // Assignment operators.
        optional&operator=(nullopt_t) noexcept {reset();return *this;}
        template<class U = T>std::enable_if_t<
            not_optional_T<U>&&T_constructible<U>&&is_assignable_v<T&, U>&&
            !(is_scalar_v<T>&&is_same_v<T, std::decay_t<U>>),
        optional&>operator=(U&& u){
            if (filled){*(T*)payload = std::forward<U>(u);}else{emplace(std::forward<U>(u));}
            return *this;
        }
        template<class U>std::enable_if_t<T_constructible<const U&>&&not_assignable_from_optional<U>,optional&>
            operator=(const optional<U>& that){
                if (that.filled){
                    if (this->filled)*(T*)(this->payload) = *(U*)that.payload;
                    else this->construct(*(U*)that.payload);
                }
                else this->reset();
                return *this;
            }
        
        template<class U>std::enable_if_t<T_constructible<U&&>&&not_assignable_from_optional<U>,optional&>
            operator=(optional<U>&& that){
                if (that.filled){
                    if (this->filled)*(T*)(this->payload) = std::move(*(U*)that.payload);
                    else this->emplace(std::move(*(U*)that.payload));
                } else this->reset();
                return *this;
            }
        template<class... Args>std::enable_if_t<T_constructible<Args&&...>, T&>
            emplace(Args&&... args)noexcept(T_nothrow_constructible<Args...>)
            { return emplace_impl(std::forward<Args>(args)...); }
        template<class U, class... Args>std::enable_if_t<T_constructible<InitList<U>&,Args&&...>, T&>
            emplace(InitList<U> il, Args&&... args)noexcept(T_nothrow_constructible<InitList<U>&, Args...>)
            { return emplace_impl(il,std::forward<Args>(args)...); }
        // Swap.
        void swap(optional& that) noexcept(is_nothrow_move_constructible_v<T> && is_nothrow_swappable_v<T>){
            using std::swap;
            if (this->filled && that.filled) swap(*(T*)(this->payload),*(T*)that.payload);
            else if (this->filled){
                that.emplace(std::move(*(T*)(this->payload)));
                this->reset();
            }
            else if (that.filled){
                this->emplace(std::move(*(T*)that.payload));
                that.reset();
            }
        }
        
        // Observers.
        constexpr const T*operator->() const{ return (T*)payload; }
        T*operator->(){ return (T*)payload; }
        constexpr const T&operator*() const&{ return *(T*)payload; }
        constexpr T&operator*()&{ return *(T*)payload; }
        constexpr T&&operator*()&&{ return std::move(*(T*)payload); }
        constexpr const T&&operator*() const&&{ return std::move(*(T*)payload); }
        constexpr bool has_value() const noexcept { return filled; }
        constexpr explicit operator bool() const noexcept{ return filled; }
        constexpr const T&value() const& {assert_filled();return *(T*)payload;}
        constexpr T&value()&{assert_filled();return *(T*)payload;}
        constexpr T&&value()&&{assert_filled();return std::move(*(T*)payload);}
        constexpr const T&&value() const&&{assert_filled();return std::move(*(T*)payload);}
        template<class Def>constexpr T value_or(Def&& def) const&{
            static_assert(is_copy_constructible_v<T>&&convertible_T<Def&&>,"Cannot return value");
            return filled? *(T*)payload: static_cast<T>(std::forward<Def>(def));
        }
        template<class Def>T value_or(Def&& def) && {
            static_assert(is_move_constructible_v<T>&&convertible_T<Def&&>,"Cannot return value" );
            return filled? std::move(*(T*)payload): static_cast<T>(std::forward<Def>(def));
        }
        using Base::reset;
    };// class optional
    namespace _opt{
        template<class T>using relop_t = std::enable_if_t<is_convertible_v<T, bool>, bool>;
        template<class T,class U>using eq_t=relop_t<decltype(std::declval<T>() == std::declval<U>())>;
        template<class T,class U>using ne_t=relop_t<decltype(std::declval<T>() != std::declval<U>())>;
        template<class T,class U>using lt_t=relop_t<decltype(std::declval<T>() <  std::declval<U>())>;
        template<class T,class U>using le_t=relop_t<decltype(std::declval<T>() <= std::declval<U>())>;
        template<class T,class U>using ge_t=relop_t<decltype(std::declval<T>() >= std::declval<U>())>;
        template<class T,class U>using gt_t=relop_t<decltype(std::declval<T>() >  std::declval<U>())>;
    }
    // Comparisons between optional values.
    template<class T, class U>constexpr _opt::eq_t<T,U> operator==(const optional<T>& lhs, const optional<U>& rhs)
    {return lhs.has_value() == rhs.has_value() && (!lhs || *lhs == *rhs);}
    template<class T, class U>constexpr _opt::ne_t<T,U> operator!=(const optional<T>& lhs, const optional<U>& rhs)
    { return lhs.has_value() != rhs.has_value() || (lhs.has_value() && *lhs != *rhs);}
    
    template<class T, class U> constexpr _opt::lt_t<T,U> operator<(const optional<T>& lhs, const optional<U>& rhs)
    { return rhs.has_value() && (!lhs || *lhs < *rhs); }
    template<class T, class U> constexpr _opt::le_t<T,U> operator<=(const optional<T>& lhs, const optional<U>& rhs)
    { return !lhs || (rhs.has_value() && *lhs <= *rhs); }
    template<class T, class U> constexpr _opt::ge_t<T,U> operator>=(const optional<T>& lhs, const optional<U>& rhs)
    { return !rhs || (lhs.has_value() && *lhs >= *rhs); }
    template<class T, class U> constexpr _opt::gt_t<T,U> operator>(const optional<T>& lhs, const optional<U>& rhs)
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
    template<class T, class U> constexpr _opt::eq_t<T,U>
        operator==(const optional<T>& lhs, const U& rhs){ return lhs && *lhs == rhs; }
    template<class T, class U> constexpr _opt::eq_t<T,U>
        operator==(const T& lhs, const optional<U>& rhs){ return rhs && lhs == *rhs; }
    template<class T, class U> constexpr _opt::ne_t<T,U>
        operator!=(const optional<T>& lhs, const U& rhs){ return !lhs || *lhs != rhs; }
    template<class T, class U> constexpr _opt::ne_t<T,U>
        operator!=(const T& lhs, const optional<U>& rhs){ return !rhs || lhs != *rhs; }
    template<class T, class U> constexpr _opt::lt_t<T,U>
        operator<(const optional<T>& lhs, const U& rhs){ return !lhs || *lhs < rhs; }
    template<class T, class U> constexpr _opt::lt_t<T,U>
        operator<(const T& lhs, const optional<U>& rhs){ return rhs && lhs < *rhs; }
    template<class T, class U> constexpr _opt::le_t<T,U>
        operator<=(const optional<T>& lhs, const U& rhs){ return !lhs || *lhs <= rhs; }
    template<class T, class U> constexpr _opt::le_t<T,U>
        operator<=(const T& lhs, const optional<U>& rhs){ return rhs && lhs <= *rhs; }
    template<class T, class U> constexpr _opt::ge_t<T,U>
        operator>=(const optional<T>& lhs, const U& rhs){ return lhs && *lhs >= rhs; }
    template<class T, class U> constexpr _opt::ge_t<T,U>
        operator>=(const T& lhs, const optional<U>& rhs){ return !rhs || lhs >= *rhs; }
    template<class T, class U> constexpr _opt::gt_t<T,U>
        operator>(const optional<T>& lhs, const U& rhs){ return lhs && *lhs > rhs; }
    template<class T, class U> constexpr _opt::gt_t<T,U>
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
    namespace _opt{
        template<class T, class U = std::remove_const_t<T>,bool=std::__poison_hash<U>::__enable_hash_call>struct hash{};
        template<class T, class U>struct hash<T, U, true> {
            size_t operator()(const optional<T>& t) const noexcept(noexcept(std::hash<U>{}(*t))){
                return t ? std::hash<U>{}(*t) : static_cast<size_t>(-3333);
            }
        };
    }
} // namespace backports

template<class T>struct std::hash<backports::optional<T>>:
private __poison_hash<std::remove_const_t<T>>,public backports::_opt::hash<T>{
        using result_type = size_t;using argument_type = backports::optional<T>;
    };
#if __cpp_deduction_guides >= 201606
template <class T> optional(T) -> optional<T>;
#endif

#endif // OPTIONAL_HPP