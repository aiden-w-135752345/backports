#ifndef VARIANT_HPP
#define VARIANT_HPP
#include "detail/special_members.hpp"
#include "detail/inline_variables.hpp"
#include "utility.hpp"
#include "functional.hpp"
#include <type_traits>
#include <initializer_list>
#include <exception>
#include <array>
namespace backports {
    template<class...Ts> class variant;
    struct monostate {};
    class bad_variant_access : public std::exception{
        const char* reason;
    public:
        bad_variant_access() noexcept : reason("Unknown reason") { }
        bad_variant_access(const char* r) : reason(r) { }
        const char* what() const noexcept override;
    };
    constexpr INLINE const size_t variant_npos = size_t(-1);
    namespace _variant {
        template<class T>using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
        template<class... Ts>struct typelist{};
        template<size_t i,class,class...Ts>struct Ith_type:Ith_type<i-1,Ts...>{};
        template<class T,class...Ts>struct Ith_type<0,T,Ts...>{using type=T;};
        template<size_t i,class...Ts>using Ith_type_t=typename Ith_type<i,Ts...>::type;
        template<bool...B>Ith_type_t<0,std::true_type,std::enable_if_t<B>...> and_fn(int);
        template<class T,class...>struct t_includes:std::false_type{};
        template<class T,class U,class...Ts>struct t_includes<T,U,Ts...>:t_includes<T,Ts...>{};
        template<class T,class...Ts>struct t_includes<T,T,Ts...>:std::true_type{};
        
        template<size_t i>using index_constant=std::integral_constant<size_t,i>;
        template<size_t i,class H,class...Hs>struct t_find_uniq{};
        template<size_t i,class N,class H,class...Hs>struct t_find_uniq<i,N,H,Hs...>:t_find_uniq<i+1,N,Hs...>{};
        template<size_t i,class N,class...Hs>struct t_find_uniq<i,N,N,Hs...>:std::enable_if<!t_includes<N,Hs...>::value,index_constant<i>>{};
        template<class N,class...Hs>constexpr INLINE const size_t t_find_uniq_v=t_find_uniq<0,N,Hs...>::type::value;
        template<bool...>std::false_type and_fn(...);
        template<bool...B>constexpr INLINE const bool and_v=decltype(and_fn<B...>(0))::value;
        template<size_t,size_t...>struct max{};
        template<size_t x>struct max<x>:index_constant<x>{};
        template<size_t x,size_t y,size_t...rest>struct max<x,y,rest...>:max<(x>y?x:y),rest...>{};
        
        template<class>struct first_alternative;
        template<class T,class...Ts>struct first_alternative<variant<T,Ts...>>{using type=T;};
        template<class T>struct first_alternative<T&>{using type=typename first_alternative<T>::type&;};
        template<class T>struct first_alternative<T&&>{using type=typename first_alternative<T>::type&&;};
        template<class T>struct first_alternative<const T>{using type=const typename first_alternative<T>::type;};
        template<class F,class...Vars>using visit_result_t=invoke_result_t<F,typename first_alternative<Vars>::type...>;
        template<class T>struct is_in_place_tag:std::false_type{};
        template<class T>struct is_in_place_tag<in_place_type_t<T>>:std::true_type {};
        template<size_t i>struct is_in_place_tag<in_place_index_t<i>>:std::true_type {};
        template<class T,class...Ts>struct accepted_index_test{
            struct unit_array{T value[1];};
            template<class U,class Arr=decltype(unit_array{{std::declval<U>()}})>
                static index_constant<t_find_uniq_v<T,Ts...,Arr>> get(in_place_type_t<U>,T);
        };
        template<class...>struct accepted_index_impl;
        template<>struct accepted_index_impl<>{static void get()=delete;};
        template<class T,class...Ts>struct accepted_index_impl<T,Ts...>:T,accepted_index_impl<Ts...>{
            using T::get;using accepted_index_impl<Ts...>::get;
        };
        template<class,class,class...>struct basic_erased;
        template<class F,size_t...is,class...Args>struct basic_erased<F,std::index_sequence<is...>,Args...>{
            using visit_result=invoke_result_t<F,index_constant<variant_npos>>;
            template<size_t i>constexpr static visit_result erased(F&&f,Args&&...args){
                std::forward<F>(f)(index_constant<i>{},std::forward<Args>(args)...);
            }
            constexpr const static std::array<visit_result(*)(F&&,Args&&...),sizeof...(is)>value={&erased<is>...};
        };
        template<size_t max,class F,class...Args>constexpr invoke_result_t<F,index_constant<variant_npos>>basic_visit(F&&f,size_t idx,Args&&...args){
            if(idx==variant_npos){return std::forward<F>(f)(index_constant<variant_npos>{},std::forward<Args>(args)...);}
            constexpr const auto erased=basic_erased<F,std::make_index_sequence<max>,Args...>::value;
            return erased[idx](std::forward<F>(f),std::forward<Args>(args)...);
        }
        template<class Variant>struct Base;
        template<class...Ts>struct Base<variant<Ts...>> {
            template<size_t i>using to_type=Ith_type_t<i,Ts...>;
            size_t idx=variant_npos;
            alignas(max<alignof(Ts)...>::value) unsigned char payload[max<sizeof(Ts)...>::value];
            constexpr Base()=default;
            template<size_t i>constexpr to_type<i>*get_as(){
                return reinterpret_cast<to_type<i>*>(payload);
            }
            template<size_t i>constexpr const to_type<i>*get_as()const{
                return reinterpret_cast<const to_type<i>*>(payload);
            }
            struct CopyCtorVisitor{
                constexpr void operator()(index_constant<variant_npos>,void*,const void*){}
                template<size_t i>constexpr void operator()(index_constant<i>,void*self,const void*other){
                    using Payload=std::remove_const_t<to_type<i>>;
                    ::new(self)Payload(*reinterpret_cast<const Payload*>(other));
                }
            };
            constexpr void construct(const Base&that)noexcept(_variant::and_v<is_nothrow_copy_constructible_v<Ts>...>){
                basic_visit<sizeof...(Ts)>(CopyCtorVisitor{},that.idx,&this->payload,&that->payload);this->idx=that.idx;
            }
            struct MoveCtorVisitor{
                constexpr void operator()(index_constant<variant_npos>,void*,void*){}
                template<size_t i>constexpr void operator()(index_constant<i>,void*self,void*other){
                    using Payload=std::remove_const_t<to_type<i>>;
                    ::new(self)Payload(std::move(*reinterpret_cast<Payload*>(other)));
                }
            };
            constexpr void construct(const Base&&that)noexcept(_variant::and_v<is_nothrow_copy_constructible_v<Ts>...>){
                basic_visit<sizeof...(Ts)>(MoveCtorVisitor{},that.idx,&this->payload,&that->payload);this->idx=that.idx;
            }
            template<size_t i,class T>constexpr void assign(in_place_index_t<i>,T&&that,std::true_type){
                using Payload=std::remove_const_t<to_type<i>>;
                if (idx == i){
                    *get_as<i>() = std::forward<T>(that);
                }else{
                    this->reset();
                    ::new(get_as<i>())Payload(std::forward<T>(that));
                    this->idx=i;
                }
            }
            template<size_t i,class T>constexpr void assign(in_place_index_t<i>,T&&that,std::false_type){
                using Payload=std::remove_const_t<to_type<i>>;
                if (this->idx == i){*get_as<i>() = std::forward<T>(that);}
                else{
                    Payload copy = Payload(std::forward<T>(that));
                    this->reset();
                    ::new(get_as<i>())Payload(std::move(copy));
                    this->idx=i;
                }
            }
            template<size_t i,class T>constexpr void assign(in_place_index_t<i>tag,T&&that){
                assign(tag,std::forward<T>(that),bool_constant<is_nothrow_constructible_v<to_type<i>,T>||!is_nothrow_move_constructible_v<to_type<i>>>{});
            }
            struct CopyAssignVisitor{
                constexpr void operator()(index_constant<variant_npos>,Base*self,const void*){self->reset();}
                template<size_t i>constexpr void operator()(index_constant<i>,Base*self,const void*other){
                    using Payload=std::remove_const_t<to_type<i>>;
                    self->assign(in_place_index<i>,*reinterpret_cast<const Payload*>(other));
                }
            };
            constexpr void assign(const Base&that)noexcept(_variant::and_v<is_nothrow_copy_constructible_v<Ts>...,is_nothrow_copy_assignable_v<Ts>...>){
                basic_visit<sizeof...(Ts)>(CopyAssignVisitor{},that.idx,this,&that->payload);
            }
            struct MoveAssignVisitor{
                constexpr void operator()(index_constant<variant_npos>,void*,void*){}
                template<size_t i>constexpr void operator()(index_constant<i>,void*self,void*other){
                    using Payload=std::remove_const_t<to_type<i>>;
                    *reinterpret_cast<Payload*>(self)=std::move(*reinterpret_cast<Payload*>(other));
                }
            };
            constexpr void assign(Base&&that)noexcept(_variant::and_v<is_nothrow_move_constructible_v<Ts>...,is_nothrow_move_assignable_v<Ts>...>){
                if(this->idx==that.idx){basic_visit<sizeof...(Ts)>(MoveAssignVisitor{},that.idx,&this->payload,&that->payload);}
                else{this->reset();this->construct(std::move(that)); }
            }
            constexpr void destruct(){reset();}
            struct ResetVisitor{
                constexpr void operator()(index_constant<variant_npos>,void*){}
                template<size_t i>constexpr void operator()(index_constant<i>,void*self){
                    using Payload=std::remove_const_t<to_type<i>>;
                    reinterpret_cast<Payload*>(self)->~Payload();
                }
            };
            constexpr void reset(){basic_visit<sizeof...(Ts)>(ResetVisitor{},idx,&this->payload);idx=variant_npos;}
        };
        template<class...Ts>using Base_t=detail::special_members<Base<variant<Ts...>>,
            and_v<is_copy_constructible_v<Ts>...>,and_v<is_trivially_copy_constructible_v<Ts>...>,
            and_v<is_move_constructible_v<Ts>...>,and_v<is_trivially_move_constructible_v<Ts>...>,
            and_v<is_copy_constructible_v<Ts>...,is_copy_assignable_v<Ts>...>,
            and_v<is_trivially_copy_constructible_v<Ts>...,is_trivially_copy_assignable_v<Ts>...>,
            and_v<is_move_constructible_v<Ts>...,is_move_assignable_v<Ts>...>,
            and_v<is_trivially_move_constructible_v<Ts>...,is_trivially_move_assignable_v<Ts>...>,
            and_v<is_trivially_destructible_v<Ts>...>>;
        template<class...Ts>constexpr Base<variant<Ts...>>&get_base(variant<Ts...>&);
        template<class...Ts>constexpr const Base<variant<Ts...>>&get_base(const variant<Ts...>&);
    }// namespace _variant
    template<size_t I,class T>struct variant_alternative;
    template<size_t I,class...Types>struct variant_alternative<I, variant<Types...>>:_variant::Ith_type<I,Types...>{};
    template<size_t I,class T>struct variant_alternative<I, const T>{using type=const typename variant_alternative<I,T>::type;};
    template<size_t I,class T>using variant_alternative_t = typename variant_alternative<I, T>::type;
    template<class T>struct variant_size;
    template<class...Types>struct variant_size<variant<Types...>>:_variant::index_constant<sizeof...(Types)>{};
    template<class T>struct variant_size<const T>:variant_size<T>{};
    template<class T>constexpr INLINE const size_t variant_size_v = variant_size<T>::value;
    template<class...Ts>class variant:private _variant::Base_t<Ts...>{
        using Base=_variant::Base_t<Ts...>;
        template<size_t i>using to_type=_variant::Ith_type_t<i,Ts...>;
        template<class T>constexpr static const bool not_self = !is_same_v<_variant::remove_cvref_t<T>, variant>;
        template<class T>constexpr static const size_t to_index = _variant::t_find_uniq_v<T, Ts...>;
        template<class T>constexpr static const bool not_in_place_tag= !_variant::is_in_place_tag<_variant::remove_cvref_t<T>>::value;
        using accepted_index_impl=_variant::accepted_index_impl<_variant::accepted_index_test<Ts,Ts...>...>;
        template<class T,class=decltype(accepted_index_impl::get(in_place_type<T>,std::declval<T>()))> constexpr static const size_t accepted_index=decltype(accepted_index_impl::get(in_place_type<T>,std::declval<T>()))::value;
        friend constexpr _variant::Base<variant>&_variant::get_base<>(variant&);
        friend constexpr const _variant::Base<variant>&_variant::get_base<>(const variant&);
        void*get(){return &(this->payload);}
        const void*get()const{return &(this->payload);}
        template <size_t i,class T,class... Args>T&emplace_impl( Args&&... args ){
            Base::reset();
            using Payload = std::remove_const_t<T>;
            Payload*payload=Base::template get_as<i>();
            ::new(payload)Payload(std::forward<Args>(args)...);
            this->idx = i;
            return *payload;
        }
    public:
        template<class = std::enable_if_t<is_default_constructible_v<to_type<0>>>>
            constexpr variant() noexcept(is_nothrow_default_constructible_v<to_type<0>>):variant(in_place_index<0>){}
        template<class T,size_t i = accepted_index<T&&>,class U = to_type<i>,class=std::enable_if_t<not_in_place_tag<T> && is_constructible_v<U,T>>>
            constexpr variant(T&& t)noexcept(is_nothrow_constructible_v<U, T>)
                :Base(){emplace<i>(std::forward<T>(t));}
        template<class T, class... Args,
            size_t i=to_index<std::enable_if_t<is_constructible_v<T, Args...>,T>>>
            constexpr explicit variant(in_place_type_t<T>, Args&&... args)
                :Base(){emplace<i>(std::forward<Args>(args)...);}
        template<class T, class U, class... Args,
            size_t i=to_index<std::enable_if_t<is_constructible_v<T,std::initializer_list<U>&, Args...>,T>>>
            constexpr explicit variant(in_place_type_t<T>, std::initializer_list<U> il,Args&&... args)
                :Base(){emplace<i>(il,std::forward<Args>(args)...);}
        template<size_t i,class...Args,class = std::enable_if_t<is_constructible_v<to_type<i>, Args...>>>
            constexpr explicit variant(in_place_index_t<i>, Args&&... args)
                :Base(){emplace<i>(std::forward<Args>(args)...);}
        template<size_t i, class U, class... Args,class=std::enable_if_t<is_constructible_v<to_type<i>,std::initializer_list<U>&, Args...>>>
            constexpr explicit variant(in_place_index_t<i>, std::initializer_list<U> il,Args&&... args)
                :Base(){emplace<i>(il, std::forward<Args>(args)...);}
        template<class T,size_t i = accepted_index<T&&>,class U=to_type<i>>std::enable_if_t<is_constructible_v<U,T>&&is_assignable_v<U&,T>,variant&> operator=(T&& that)
            noexcept(is_nothrow_assignable_v<U&, T>&&is_nothrow_constructible_v<U, T>){this->assign(in_place_index<i>,that);return *this;}
        template<class T, class... Args,size_t i=to_index<T>>std::enable_if_t<is_constructible_v<T, Args...>,T&>
            emplace(Args&&... args){return emplace<i>(std::forward<Args>(args)...);}
        template<class T, class U, class... Args,size_t i=to_index<T>>
            std::enable_if_t<is_constructible_v<T,std::initializer_list<U>&, Args...>,T&>
            emplace(std::initializer_list<U> il, Args&&... args){
                return emplace<i>(il, std::forward<Args>(args)...);
            }
        template <size_t i,class... Args, class T=to_type<i>>
            std::enable_if_t<is_constructible_v<T,Args...>,T&>
            emplace( Args&&... args ){return emplace_impl<i,T>(std::forward<Args>(args)...);}
        template <size_t i,class U, class... Args, class T=to_type<i>>
            std::enable_if_t<is_constructible_v<T,std::initializer_list<U>,Args...>,T&>
            emplace( std::initializer_list<U> il, Args&&... args ){return emplace_impl<i,T>(il,std::forward<Args>(args)...);}
        constexpr bool valueless_by_exception() const noexcept{return this->idx==variant_npos;}
        constexpr size_t index() const noexcept{return this->idx;}
        // swap
    };
    namespace _variant{
        template<class...Ts>constexpr Base<variant<Ts...>>&get_base(variant<Ts...>&v){return v;}
        template<class...Ts>constexpr const Base<variant<Ts...>>&get_base(const variant<Ts...>&v){return v;}
        [[noreturn]]inline void throw_access(bool valueless){
            throw bad_variant_access(valueless?"variant is valueless":"wrong index for variant");
        }
    }// namespace _variant
    template<class T,class...Ts>constexpr bool holds_alternative(const variant<Ts...>&v){
        return v.index()==_variant::t_find_uniq_v<T, Ts...>;
    }
    template<size_t i,class...Ts>constexpr _variant::Ith_type_t<i,Ts...>&get(variant<Ts...>& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return *_variant::get_base(v).template get_as<i>();
    }
    template<size_t i,class...Ts>constexpr _variant::Ith_type_t<i,Ts...>&&get(variant<Ts...>&& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return std::move(*_variant::get_base(v).template get_as<i>());
    }
    template<size_t i,class...Ts>constexpr const _variant::Ith_type_t<i,Ts...>&get(const variant<Ts...>& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return *_variant::get_base(v).template get_as<i>();
    }
    template<size_t i,class...Ts>constexpr const _variant::Ith_type_t<i,Ts...>&&get(const variant<Ts...>&& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return std::move(*_variant::get_base(v).template get_as<i>());
    }
    template<class T,class...Ts>constexpr T& get(variant<Ts...>& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    template<class T,class...Ts>constexpr T&& get(variant<Ts...>&& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    template<class T,class...Ts>constexpr const T& get(const variant<Ts...>& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    template<class T,class...Ts>constexpr const T&& get(const variant<Ts...>&& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    
    template<size_t i,class...Ts>constexpr _variant::Ith_type_t<i,Ts...>* get_if(variant<Ts...>* pv)
        noexcept{return pv&&pv->index()==i?_variant::get_base(pv).template get_as<i>():nullptr;}
    template<size_t i,class...Ts>constexpr const _variant::Ith_type_t<i,Ts...>*get_if(const variant<Ts...>* pv)
        noexcept{return pv&&pv->index()==i?_variant::get_base(pv).template get_as<i>():nullptr;}
    template<class T,class...Ts>constexpr T*get_if(variant<Ts...>* pv) noexcept{
        return get_if<_variant::t_find_uniq_v<T, Ts...>>(pv);
    }
    template<class T,class...Ts>constexpr const T*get_if(const variant<Ts...>* pv) noexcept{
        return get_if<_variant::t_find_uniq_v<T, Ts...>>(pv);
    }
    namespace _variant{
#define RELATION_TEMPLATE(name,OP) \
        template<class...Ts>struct name##visitor{ \
            constexpr bool operator()(index_constant<variant_npos>,const void*,const void*){ return 0 OP 0; } \
            template<size_t i>constexpr bool operator()(index_constant<i>,const void*lhs,const void*rhs){ \
                using Payload=std::remove_const_t<_variant::Ith_type_t<i,Ts...>>; \
                return *reinterpret_cast<const Payload*>(lhs) OP *reinterpret_cast<const Payload*>(rhs); \
            } \
        }
        RELATION_TEMPLATE(lt,<);
        RELATION_TEMPLATE(le,<=);
        RELATION_TEMPLATE(eq,==);
        RELATION_TEMPLATE(ne,!=);
        RELATION_TEMPLATE(ge,>=);
        RELATION_TEMPLATE(gt,>);
#undef RELATION_TEMPLATE
    }// namespace _variant
#define RELATION_TEMPLATE(name,OP) \
    template<class...Ts>constexpr bool operator OP(const variant<Ts...>& lhs,const variant<Ts...>& rhs) { \
        if (lhs.index() != rhs.index()) return (lhs.index()+1) OP (rhs.index()+1); \
        return _variant::basic_visit<sizeof...(Ts)>(_variant::name##visitor<Ts...>{},lhs.index(),&_variant::get_base(lhs).payload,&_variant::get_base(rhs).payload); \
    }
    RELATION_TEMPLATE(lt,<)
    RELATION_TEMPLATE(le,<=)
    RELATION_TEMPLATE(eq,==)
    RELATION_TEMPLATE(ne,!=)
    RELATION_TEMPLATE(ge,>=)
    RELATION_TEMPLATE(gt,>)
#undef RELATION_TEMPLATE
    namespace _variant{
        template<class... Ts>constexpr variant<Ts...>&as_variant(variant<Ts...>& v) noexcept{ return v; }
        template<class... Ts>constexpr const variant<Ts...>&as_variant(const variant<Ts...>& v) noexcept{ return v; }
        template<class... Ts>constexpr variant<Ts...>&&as_variant(variant<Ts...>&& v) noexcept{ return std::move(v); }
        template<class... Ts>constexpr const variant<Ts...>&&as_variant(const variant<Ts...>&& v) noexcept{ return std::move(v); }
        template<class>struct voidptr;
        template<class...Ts>struct voidptr<const variant<Ts...>>{using type=const void*;};
        template<class...Ts>struct voidptr<variant<Ts...>>{using type=void*;};
        template<class Var>using voidptr_t=typename voidptr<std::remove_reference_t<Var>>::type;
        template<class,class,class,class>struct combine_erased_impl;
        template<class T,class U,size_t...is,size_t...js>struct combine_erased_impl<T,U,std::index_sequence<is...>,std::index_sequence<js...>>{
            using value_type=typename T::value_type;
            constexpr static std::array<value_type,T::value.size()+U::value.size()>value={T::value[is]...,U::value[js]...};
        };
        template<class... Ts>struct combine_erased;
        template<class T>struct combine_erased<T>:T{};
        template<class T,class U,class...Ts>struct combine_erased<T,U,Ts...>
            :combine_erased<combine_erased_impl<T,U,std::make_index_sequence<T::value.size()>,std::make_index_sequence<U::value.size()>>,Ts...>{};
        template<class,class,class>struct create_erased;
        template<class R,class F,class...Vs,class...Args>struct create_erased<R(*)(F,Vs...),typelist<Args...>,typelist<>>{
            using value_type=R(*)(F,Vs...);
            constexpr static R erased(F f,Vs...v){
                return invoke(std::forward<F>(f),std::forward<Args>(*reinterpret_cast<std::remove_reference_t<Args>*>(v))...);
            }
            constexpr static std::array<const value_type,1>value={&erased};
        };
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<variant<Alts...>,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,const Alts&>,typelist<Vars...>>...>{};
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<variant<Alts...>&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,Alts&>,typelist<Vars...>>...>{};
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<const variant<Alts...>&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,const Alts&>,typelist<Vars...>>...>{};
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<variant<Alts...>&&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,Alts&&>,typelist<Vars...>>...>{};
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<const variant<Alts...>&&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,const Alts&&>,typelist<Vars...>>...>{};
        template<class F,class...Vars>constexpr auto visit(F&& f,Vars&&...vars){
            constexpr size_t sizes[]={variant_size_v<remove_cvref_t<Vars>>...};
            size_t indeces[]={vars.index()...};
            size_t index=0;
            for(size_t i=0;i<sizeof...(Vars);i++){
                if(indeces[i]==variant_npos){throw_access(true);}
                index*=sizes[i];index+=indeces[i];
            }
            constexpr auto erased=create_erased<visit_result_t<F,Vars...>(*)(F&&,voidptr_t<Vars>...),typelist<>,typelist<Vars...>>::value;
            return erased[index](std::forward<F>(f),&get_base(vars).payload...);
        }
    } // namespace _variant
    template<class F,class...Vars>constexpr _variant::visit_result_t<F,decltype(_variant::as_variant(std::declval<Vars>()))...>
        visit(F&& f,Vars&&...vars){return _variant::visit(std::forward<F>(f),_variant::as_variant(std::forward<Vars>(vars))...);}
    namespace _variant{
        template<class...Ts>struct hashVisitor{
            constexpr size_t operator()(index_constant<variant_npos>,void*){return 0;}
            template<size_t i>constexpr size_t operator()(index_constant<i>,const void*self){
                using Payload=std::remove_const_t<_variant::Ith_type_t<i,Ts...>>;
                return std::hash<Payload>{}(*reinterpret_cast<const Payload*>(self));
            }
        };
        template<class T, class=void>class hash{
            private: // Private rather than deleted to be non-trivially-copyable.
            hash(hash&&);~hash();
        };
        template<class...Ts>struct hash<variant<Ts...>, void_t<decltype(std::hash<Ts>()(std::declval<Ts>()))...>> {
            size_t operator()(const variant<Ts...>& that) const 
                noexcept(and_v<is_nothrow_invocable_v<std::hash<std::remove_const_t<Ts>>,Ts>...>){
                    return std::hash<size_t>{}(that.index())+basic_visit<sizeof...(Ts)>(hashVisitor<Ts...>{},that.index,&get_base(that).payload);
                }

        };
    }
}//namespace backports
template<class...Ts>struct std::hash<backports::variant<Ts...>>:public backports::_variant::hash<backports::variant<Ts...>>{};
template<>struct std::hash<backports::monostate>{
    using argument_type = backports::monostate;
    using result_type = size_t;
    size_t operator()(const backports::monostate&) const noexcept{return size_t(-7777);}
};
#endif //VARIANT_HPP
