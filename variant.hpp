#ifndef VARIANT_HPP
#define VARIANT_HPP
#include "detail/special_members.hpp"
#include "utility.hpp"
#include "functional.hpp"
#include "inline_variables.hpp"
#include <type_traits>
#include <utility>
#include <initializer_list>
#include <exception>
namespace backports {
    template<class...Ts> class variant;
    struct monostate {};
    class bad_variant_access : public std::exception{
        const char* reason;
    public:
        bad_variant_access() noexcept : reason("Unknown reason") { }
        bad_variant_access(const char* reason) : reason(reason) { }
        const char* what() const noexcept override{ return reason; }
    };
    constexpr INLINE const std::size_t variant_npos = -1;
    namespace _variant {
        template<class T>using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
        template<size_t i,class,class...Ts>struct Ith_type:Ith_type<i-1,Ts...>{};
        template<class T,class...Ts>struct Ith_type<0,T,Ts...>{using type=T;};
        template<size_t i,class...Ts>using Ith_type_t=typename Ith_type<i,Ts...>::type;
        template<bool...B>Ith_type_t<0,std::true_type,std::enable_if_t<B>...> and_fn(int);
        
        
        template<class T,class...>struct t_includes:std::false_type{};
        template<class T,class U,class...Ts>struct t_includes<T,U,Ts...>:t_includes<T,Ts...>{};
        template<class T,class...Ts>struct t_includes<T,T,Ts...>:std::true_type{};
        
        template<size_t i,class H,class...Hs>struct t_find_uniq{};
        template<size_t i,class N,class H,class...Hs>struct t_find_uniq<i,N,H,Hs...>:t_find_uniq<i+1,N,Hs...>{};
        template<size_t i,class N,class...Hs>struct t_find_uniq<i,N,N,Hs...>:std::enable_if<!t_includes<N,Hs...>::value,std::integral_constant<size_t,i>>{};
        template<class N,class...Hs>constexpr INLINE const size_t t_find_uniq_v=t_find_uniq<0,N,Hs...>::type::value;
        
        template<bool...>std::false_type and_fn(...);
        template<bool...B>constexpr INLINE const bool and_v=decltype(and_fn<B...>(0))::value;
        template<class T>struct is_in_place_tag:std::false_type{};
        template<class T>struct is_in_place_tag<in_place_type_t<T>>:std::true_type {};
        template<size_t i>struct is_in_place_tag<in_place_index_t<i>>:std::true_type {};
        template<class T,class...Ts>struct accepted_index_test{
            struct unit_array{T value[1];};
            // static void get(...);
            template<class U,class Arr=decltype(unit_array{{std::declval<U>()}})>
                static std::integral_constant<size_t, t_find_uniq_v<T,Ts...,Arr>> get(in_place_type_t<U>,T);
        };
        template<class T,bool already_trivial>struct force_trivial_dtor{
            ~force_trivial_dtor() {}
            force_trivial_dtor(const force_trivial_dtor&) = default;
            force_trivial_dtor(force_trivial_dtor&&) = default;
            force_trivial_dtor& operator=(const force_trivial_dtor&) = default;
            force_trivial_dtor& operator=(force_trivial_dtor&&) = default;
            union {monostate ignored;T t;};
        };
        template<class T>struct force_trivial_dtor<T,true>{T t;};
        template<class... _Types>union variadic_union{
            variadic_union() = default;
        };
        template<class First, class...Rest>union variadic_union<First, Rest...>{
            variadic_union<Rest...>rest;
            force_trivial_dtor<First,is_trivially_destructible_v<First>>first;
            constexpr variadic_union():rest(){}
        };
        template<size_t...is,class F,class...Args>void integer_visit(std::index_sequence<is...>,size_t i,F&&f,Args&&...args){
            int ignored[]={(i==is?(std::forward<F>(f)(std::integral_constant<size_t,is>{},std::forward<Args>(args)...),0):0)...};
            (void)ignored;
        }
        template<class Variant>struct Base;
        template<class...Ts>struct Base<variant<Ts...>> {
            template<size_t i>using to_type=Ith_type_t<i,Ts...>;
            size_t idx=variant_npos;
            variadic_union<Ts...> payload;
            constexpr Base()=default;
            constexpr void construct(const Base&that)noexcept(_variant::and_v<is_nothrow_copy_constructible_v<Ts>...>){
                integer_visit(std::index_sequence_for<Ts...>{},that.idx,[](auto idx,void*self,const void*that){
                    using Payload=std::remove_const_t<to_type<decltype(idx)::value>>;
                    ::new(self)Payload(*(const Payload*)that);
                },(void*)&this->payload,(const void*)&that.payload);this->idx=that.idx;
            }
            constexpr void construct(const Base&&that)noexcept(_variant::and_v<is_nothrow_copy_constructible_v<Ts>...>){
                integer_visit(std::index_sequence_for<Ts...>{},that.idx,[](auto idx,void*self,void*that){
                    using Payload=std::remove_const_t<to_type<decltype(idx)::value>>;
                    ::new(self)Payload(std::move(*(Payload*)that));
                },(void*)&this->payload,(void*)&that.payload);this->idx=that.idx;
            }
            template<size_t i,class T>constexpr void assign(T&&that,std::true_type){
                using Payload=std::remove_const_t<to_type<i>>;
                if (idx == i){
                    *(Payload*)(void*)(&this->payload) = std::forward<T>(that);
                }else{
                    this->reset(); ::new((void*)&this->payload)Payload(std::forward<T>(that)); this->idx = i;
                }
            }
            template<size_t i,class T>constexpr void assign(T&&that,std::false_type){
                using Payload=std::remove_const_t<to_type<i>>;
                if (this->idx == i){*(Payload*)(void*)(&this->payload) = std::forward<T>(that);}
                else{
                    Payload copy = Payload(std::forward<T>(that));
                    this->reset(); ::new((void*)&this->payload)Payload(std::move(copy)); this->idx = i;
                }
            }
            template<size_t i,class T>constexpr void assign(T&&that){
                assign(that,bool_constant<is_nothrow_constructible_v<to_type<i>,T>||!is_nothrow_move_constructible_v<to_type<i>>>{});
            }
            constexpr void assign(const Base&that)noexcept(_variant::and_v<is_nothrow_copy_constructible_v<Ts>...,is_nothrow_copy_assignable_v<Ts>...>){
                integer_visit(std::index_sequence_for<Ts...>{},that.idx,[](auto idx,Base*self,const void*that){
                    self->assign<decltype(idx)::value>(*(const to_type<decltype(idx)::value>*)that);
                },this,(const void*)&that.payload);
                if(that.idx==variant_npos){this->reset();}
            }
            constexpr void assign(Base&&that)noexcept(_variant::and_v<is_nothrow_move_constructible_v<Ts>...,is_nothrow_move_assignable_v<Ts>...>){
                if (this->idx==that.idx){integer_visit(std::index_sequence_for<Ts...>{},that.idx,[](auto idx,void*self,void*that){
                    using Payload=std::remove_const_t<to_type<decltype(idx)::value>>;
                    *(Payload*)self=*std::move(*(Payload*)that);
                },(void*)&this->payload,(void*)&that.payload);}else{this->reset();this->construct(std::move(that)); }
            }
            constexpr void destruct(){reset();}
            constexpr void reset(){integer_visit(std::index_sequence_for<Ts...>{},idx,[](auto idx,void*payload){
                using Payload=std::remove_const_t<to_type<decltype(idx)::value>>;
                ((Payload*)payload)->~Payload();
            },(void*)&payload);idx=variant_npos;}
        };
        template<class...Ts>using Base_t=detail::special_members<Base<variant<Ts...>>,
            and_v<is_copy_constructible_v<Ts>...>,and_v<is_trivially_copy_constructible_v<Ts>...>,
            and_v<is_move_constructible_v<Ts>...>,and_v<is_trivially_move_constructible_v<Ts>...>,
            and_v<is_copy_constructible_v<Ts>...,is_copy_assignable_v<Ts>...>,
            and_v<is_trivially_copy_constructible_v<Ts>...,is_trivially_copy_assignable_v<Ts>...>,
            and_v<is_move_constructible_v<Ts>...,is_move_assignable_v<Ts>...>,
            and_v<is_trivially_move_constructible_v<Ts>...,is_trivially_move_assignable_v<Ts>...>,
            and_v<is_trivially_destructible_v<Ts>...>>;
        template<class...Ts>constexpr void*get(const variant<Ts...>&&);
        template<class...Ts>constexpr void*get(variant<Ts...>&);
        template<class...Ts>constexpr void*get(variant<Ts...>&&);
    }// namespace _variant
    template<size_t I,class T>struct variant_alternative;
    template<size_t I,class...Types>struct variant_alternative<I, variant<Types...>>:_variant::Ith_type<I,Types...>{};
    template<size_t I,class T>struct variant_alternative<I, const T>{using type=const typename variant_alternative<I,T>::type;};
    template<size_t I,class T>using variant_alternative_t = typename variant_alternative<I, T>::type;
    template<class T>struct variant_size;
    template<class...Types>struct variant_size<variant<Types...>>:std::integral_constant<size_t,sizeof...(Types)>{};
    template<class T>struct variant_size<const T>:variant_size<T>{};
    template<class T>constexpr INLINE const size_t variant_size_v = variant_size<T>::value;
    template<class...Ts>class variant:private _variant::Base_t<Ts...>{
        using Base=_variant::Base_t<Ts...>;
        friend constexpr void*_variant::get<Ts...>(const variant&&);
        friend constexpr void*_variant::get<Ts...>(variant&);
        friend constexpr void*_variant::get<Ts...>(variant&&);
        template<size_t i>using to_type=_variant::Ith_type_t<i,Ts...>;
        template<class T>constexpr static const bool not_self = !is_same_v<_variant::remove_cvref_t<T>, variant>;
        template<class T>constexpr static const size_t to_index = _variant::t_find_uniq_v<T, Ts...>;
        template<class T>constexpr static bool not_in_place_tag= !_variant::is_in_place_tag<_variant::remove_cvref_t<T>>::value;
        
        struct accepted_index_impl:_variant::accepted_index_test<Ts,Ts...>...{};
        template<class T>constexpr static const size_t accepted_index=
            decltype(accepted_index_impl::get(in_place_type<T>,std::declval<T>()))::value;
        
        template <size_t i,class T,class... Args>T&emplace_impl( Args&&... args ){
            Base::reset();
            using Payload = std::remove_const_t<T>;
            Payload*payload=(Payload*)(void*)&(this->payload);
            ::new(payload)Payload(std::forward<Args>(args)...);
            this->idx = i;
            return *payload;
        }
    public:
        template<class = std::enable_if_t<is_default_constructible_v<to_type<0>>>>
            constexpr variant() noexcept(is_nothrow_default_constructible_v<to_type<0>>):variant(in_place_index<0>){}
        template<class T,size_t i = accepted_index<std::enable_if_t<not_self<T>&&not_in_place_tag<T>,T&&>>,class U = to_type<i>,class=std::enable_if_t<not_in_place_tag<T> && is_constructible_v<U,T>>>
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
            noexcept(is_nothrow_assignable_v<U&, T>&&is_nothrow_constructible_v<U, T>){this->assign<i>(that);return *this;}
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
        template<class...Ts>constexpr const void*get(const variant<Ts...>&v){return (const void*)&(v.payload);}
        template<class...Ts>constexpr void*get(variant<Ts...>&v){return (void*)&(v.payload);}
        template<class...Ts>constexpr const void*get(const variant<Ts...>&&v){return (const void*)&(v.payload);}
        template<class...Ts>constexpr void*get(variant<Ts...>&&v){return (void*)&(v.payload);}
        inline void throw_access(bool valueless){
            throw bad_variant_access(valueless?"variant is valueless":"wrong index for variant");
        }
    }// namespace _variant
    template<class T,class...Ts>constexpr bool holds_alternative(const variant<Ts...>&v){
        return v.index()==_variant::t_find_uniq_v<T, Ts...>;
    }
    template<size_t i,class...Ts>constexpr _variant::Ith_type_t<i,Ts...>&get(variant<Ts...>& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return *(_variant::Ith_type_t<i,Ts...>*)_variant::get(v);
    }
    template<size_t i,class...Ts>constexpr _variant::Ith_type_t<i,Ts...>&&get(variant<Ts...>&& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return std::move(*(_variant::Ith_type_t<i,Ts...>*)_variant::get(v));
    }
    template<size_t i,class...Ts>constexpr const _variant::Ith_type_t<i,Ts...>&get(const variant<Ts...>& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return *(const _variant::Ith_type_t<i,Ts...>*)_variant::get(v);
    }
    template<size_t i,class...Ts>constexpr const _variant::Ith_type_t<i,Ts...>&&get(const variant<Ts...>&& v){
        if(v.index()!=i)_variant::throw_access(v.valueless_by_exception());
        return std::move(*(const _variant::Ith_type_t<i,Ts...>*)_variant::get(v));
    }
    template<class T,class...Ts>constexpr T& get(variant<Ts...>& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    template<class T,class...Ts>constexpr T&& get(variant<Ts...>&& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    template<class T,class...Ts>constexpr const T& get(const variant<Ts...>& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    template<class T,class...Ts>constexpr const T&& get(const variant<Ts...>&& v){return get<_variant::t_find_uniq_v<T, Ts...>>(v);}
    
    template<size_t i,class...Ts>constexpr _variant::Ith_type_t<i,Ts...>* get_if(variant<Ts...>* pv)
        noexcept{return pv&&pv->index()==i?(_variant::Ith_type_t<i,Ts...>*)_variant::get(*pv):nullptr;}
    template<size_t i,class...Ts>constexpr const _variant::Ith_type_t<i,Ts...>*get_if(const variant<Ts...>* pv)
        noexcept{return pv&&pv->index()==i?(const _variant::Ith_type_t<i,Ts...>*)_variant::get(*pv):nullptr;}
    template<class T,class...Ts>constexpr T*get_if(variant<Ts...>* pv) noexcept{
        return get_if<_variant::t_find_uniq_v<T, Ts...>>(pv);
    }
    template<class T,class...Ts>constexpr const T*get_if(const variant<Ts...>* pv) noexcept{
        return get_if<_variant::t_find_uniq_v<T, Ts...>>(pv);
    }
    namespace _variant{
        template<class... Ts>constexpr variant<Ts...>&as_variant(variant<Ts...>& v) noexcept{ return v; }
        template<class... Ts>constexpr const variant<Ts...>&as_variant(const variant<Ts...>& v) noexcept{ return v; }
        template<class... Ts>constexpr variant<Ts...>&&as_variant(variant<Ts...>&& v) noexcept{ return std::move(v); }
        template<class... Ts>constexpr const variant<Ts...>&&as_variant(const variant<Ts...>&& v) noexcept{ return std::move(v); }
        template<class>struct first_alternative;
        template<class T,class...Ts>struct first_alternative<variant<T,Ts...>&>{using type=T&;};
        template<class T,class...Ts>struct first_alternative<const variant<T,Ts...>&>{using type=const T&;};
        template<class T,class...Ts>struct first_alternative<variant<T,Ts...>&&>{using type=T&&;};
        template<class T,class...Ts>struct first_alternative<const variant<T,Ts...>&&>{using type=const T&&;};
        template<class F,class...Variants>using visit_result_t=invoke_result_t<F,typename first_alternative<Variants>::type...>;
        template<class... Ts>struct typelist{};
        template<class,class,class,class>struct combine_erased_impl;
        template<class T,class U,size_t...is,size_t...js>struct combine_erased_impl<T,U,std::index_sequence<is...>,std::index_sequence<js...>>{
            constexpr static std::array<typename decltype(T::value)::value_type,T::value.size()+U::value.size()>value={T::value[is]...,U::value[js]...};
        };
        template<class... Ts>struct combine_erased;
        template<class T>struct combine_erased<T>:T{};
        template<class T,class U,class...Ts>struct combine_erased<T,U,Ts...>
            :combine_erased<combine_erased_impl<T,U,std::make_index_sequence<T::value.size()>,std::make_index_sequence<U::value.size()>>,Ts...>{};
        template<class,class,class>struct create_erased;
        template<class R,class F,class...Vs,class...Args>struct create_erased<R(*)(F,Vs...),typelist<Args...>,typelist<>>{
            typedef R(*ptr_t)(F,Vs...);
            constexpr static R erased(F f,Vs...v){
                return invoke(std::forward<F>(f),std::forward<Args>(*(std::remove_reference_t<Args>*)v)...);
            }
            constexpr static std::array<const ptr_t,1>value={&erased};
        };
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<variant<Alts...>&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,Alts&>,typelist<Vars...>>...>{};
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<const variant<Alts...>&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,const Alts&>,typelist<Vars...>>...>{};
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<variant<Alts...>&&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,Alts&&>,typelist<Vars...>>...>{};
        template<class...Args,class...Alts,class...Vars,class F>struct create_erased<F,typelist<Args...>,typelist<const variant<Alts...>&&,Vars...>>
            :combine_erased<create_erased<F,typelist<Args...,const Alts&&>,typelist<Vars...>>...>{};
        template<class F,class...Vars>constexpr auto visit(F&& f,Vars&&...vars){
            size_t sizes[]={variant_size_v<remove_cvref_t<Vars>>...};
            size_t indeces[]={vars.index()...};
            size_t index=0;
            for(size_t i=0;i<sizeof...(Vars);i++){index*=sizes[i];index+=indeces[i];}
            constexpr auto erased=create_erased<visit_result_t<F,Vars...>(*)(F&&,decltype(get(vars))...),typelist<>,typelist<Vars...>>::value;
            return erased[index](std::forward<F>(f),get(vars)...);
        }
    } // namespace _variant
    template<class F,class...Vars>constexpr _variant::visit_result_t<F,Vars...> visit(F&& f,Vars&&...vars){
        return _variant::visit(std::forward<F>(f),_variant::as_variant(std::forward<Vars>(vars))...);
    }
/*    
#define RELATION_FUNCTION_TEMPLATE(OP) \
    template<class A,class B>constexpr bool operator OP(const variant<A,B>& lhs,const variant<A,B>& rhs) { \
        if (lhs.index() != rhs.index()) \
            return lhs.idx OP rhs.idx; \
        if(lhs.index()==0){ \
            return lhs.get_A() OP rhs.get_A(); \
        }else if(lhs.index()==1){ \
            return lhs.get_B() OP rhs.get_B(); \
        }else{return 0 OP 0;}\
    }
    RELATION_FUNCTION_TEMPLATE(<)
    RELATION_FUNCTION_TEMPLATE(<=)
    RELATION_FUNCTION_TEMPLATE(==)
    RELATION_FUNCTION_TEMPLATE(!=)
    RELATION_FUNCTION_TEMPLATE(>=)
    RELATION_FUNCTION_TEMPLATE(>)
#undef RELATION_FUNCTION_TEMPLATE
 */
}//namespace backports
/*template<class A,class  B>struct std::hash<backports::variant<A,B>>{
    using argument_type = backports::variant<A,B>;
    using result_type = size_t;
    size_t operator()(const backports::variant<A,B>& t) const noexcept
        ((backports::is_nothrow_invocable_v<std::hash<std::decay_t<A>>,const A&>&&backports::is_nothrow_invocable_v<std::hash<std::decay_t<B>>,const A&>)){
        if(t.index()==0){return std::hash<size_t>{}(0)+std::hash<std::decay_t<A>>{}(t.get_A());}
        if(t.index()==1){return std::hash<size_t>{}(1)+std::hash<std::decay_t<B>>{}(t.get_B());}
        return std::hash<size_t>{}(-1);
    }
};*/
#endif //VARIANT_HPP
