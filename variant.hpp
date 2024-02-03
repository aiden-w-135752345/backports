#include "utility.hpp"
#include "functional.hpp"
#include <type_traits>
#include <utility>
#include <cstddef>
#include <exception>
#ifndef VARIANT_HPP
#define VARIANT_HPP
#ifndef CONST_INLINE
#define CONST_INLINE const static
#endif
namespace backports {
    class bad_variant_access : public std::exception{
        const char* reason;
    public:
        bad_variant_access() noexcept : reason("Unknown reason") { }
        bad_variant_access(const char* reason) : reason(reason) { }
        const char* what() const noexcept override{ return reason; }
    };
    template<class...Ts> class variant;
    template<class F>constexpr decltype(auto)visit(F&& visitor){return invoke(std::forward<F>(visitor));}
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,const variant<Ts...>&first,Rest&&...rest);
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,variant<Ts...>&first,Rest&&...rest);
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,variant<Ts...>&&first,Rest&&...rest);
    namespace detail {
        template<size_t i,class,class...Ts>struct to_type_impl:to_type_impl<i-1,Ts...>{};
        template<class T,class...Ts>struct to_type_impl<0,T,Ts...>{using type=T;};
        //template<class...>struct to_index_impl:std::integral_constant<-1>{};
        //template<class T,class T1,class...Ts>struct to_index_impl<T,T1,Ts...>:std::integral_constant<to_index_impl<T,Ts...>::value+1>{};
        //template<class T,class...Ts>struct to_index_impl<T,T,Ts...>:std::integral_constant<0>{};
        template<class T,class...Ts>using first=T;
        template<bool...B>first<std::true_type,std::enable_if_t<B>...> and_fn(int);
        template<bool...>std::false_type and_fn(...);
        template<bool...B>constexpr static bool and_v=decltype(and_fn<B...>(0))::value;
        /*
        template<bool cc,bool tcc,bool mc,bool tmc,bool ca,bool tca,bool ma,bool tma,bool td,class...Ts>struct Base;
#define TEMPLATE(CC,TCC,MC,TMC,CA,TCA,MA,TMA,TD,COPYCTOR,MOVECTOR,COPYASSIGN,MOVEASSIGN,DTOR) \
    template <class...Ts>struct Base<CC,TCC,MC,TMC,CA,TCA,MA,TMA,TD,Ts...> { \
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
        template<class...Ts>using Base_t=Base<
            and_v<is_copy_constructible_v<Ts>...>,and_v<is_trivially_copy_constructible_v<Ts>...>,
            and_v<is_move_constructible_v<Ts>...>,and_v<is_trivially_move_constructible_v<Ts>...>,
            and_v<is_copy_constructible_v<Ts>...,is_copy_assignable_v<Ts>...>,
            and_v<is_trivially_copy_constructible_v<Ts>...,is_trivially_copy_assignable_v<Ts>...>,
            and_v<is_move_constructible_v<Ts>...,is_move_assignable_v<Ts>...>,
            and_v<is_trivially_move_constructible_v<Ts>...,is_trivially_move_assignable_v<Ts>...>,
            and_v<is_trivially_destructible_v<Ts>...>,Ts...>;
        */
        template<class...Ts>struct template_union{alignas(Ts...) unsigned char bytes[std::max({sizeof(Ts)...})];};
        template<class T>struct erase_copyC{
            static void erased(void*dest,const void*src){new (dest)T(*(const T*)src);}
        };
        template<class T>struct erase_moveC{
            static void erased(void*dest,void*src){new (dest)T(std::move(*(T*)src));}
        };
        template<class T>struct erase_copyA{
            static void erased(void*dest,const void*src){*(T*)dest=*(const T*)src;}
        };
        template<class T>struct erase_moveA{
            static void erased(void*dest,void*src){*(T*)dest=std::move(*(T*)src);}
        };
        template<class T>struct erase_dtor{
            static void erased(void*obj){((T*)obj)->~T();}
        };
    }// namespace detail
    template<class...Ts>class variant{
        constexpr static const bool all_nothrow_copyC=detail::and_v<is_nothrow_copy_constructible_v<Ts>...>;
        constexpr static const bool all_nothrow_moveC=detail::and_v<is_nothrow_move_constructible_v<Ts>...>;
        constexpr static const bool all_nothrow_copyA=detail::and_v<is_nothrow_copy_assignable_v<Ts>...>;
        constexpr static const bool all_nothrow_moveA=detail::and_v<is_nothrow_move_assignable_v<Ts>...>;
        constexpr static const bool all_trivial_dtor =detail::and_v<is_trivially_destructible_v <Ts>...>;
        detail::template_union<Ts...> _M_u;
        unsigned char idx;
        void construct_impl(const variant&that){
            constexpr void(*const erased[])(void *, const void *)={&detail::erase_copyC<Ts>::erased...};
            erased[idx](ptr(),that.ptr());
        }
        void construct_impl(variant&& that){
            constexpr void(*const erased[])(void *,void *)={&detail::erase_moveC<Ts>::erased...};
            erased[idx](ptr(),that.ptr());
        }
        void destruct(){
            constexpr void(*const erased[])(void *)={&detail::erase_dtor<Ts>::erased...};
            erased[idx](ptr());
        }
        inline constexpr void* ptr(){return &_M_u;}
        inline constexpr const void* ptr()const{return &_M_u;}
        template<class F,class...Rest>friend constexpr decltype(auto)visit(F&& visitor,const variant&first,Rest&&...rest);
        template<class F,class...Rest>friend constexpr decltype(auto)visit(F&& visitor,variant&first,Rest&&...rest);
        template<class F,class...Rest>friend constexpr decltype(auto)visit(F&& visitor,variant&&first,Rest&&...rest);
    public:
        template<size_t i>using to_type=typename detail::to_type_impl<i,Ts...>::type;
        template<class = std::enable_if_t<std::is_default_constructible<to_type<0>>::value>>
            constexpr variant() noexcept(std::is_nothrow_default_constructible<to_type<0>>::value):variant(in_place_index<0>){}
        template<size_t i,class... Args,class = std::enable_if_t<std::is_constructible<to_type<i>,Args...>::value>>
            constexpr explicit variant(in_place_index_t<i>, Args&&... args) noexcept(std::is_nothrow_constructible<to_type<i>,Args...>::value)
                :idx(i){::new (ptr()) to_type<i>(std::forward<Args>(args)...);}
        constexpr variant(const variant&that)noexcept(all_nothrow_copyC):_M_u(),idx(that.idx){construct_impl(that);}
        constexpr variant(variant&&that)noexcept(all_nothrow_moveC):_M_u(),idx(that.idx){construct_impl(std::move(that));}
        ~variant(){destruct();};
        variant&operator=(const variant&that)noexcept(all_nothrow_copyC&&all_nothrow_copyA){
            if(idx==that.idx){
                constexpr void(*const erased[])(void *, const void *)={&detail::erase_copyA<Ts>::erased...};
                erased[idx](ptr(),that.ptr());
            }else{
                destruct();idx=-1;construct_impl(that);idx=that.idx;
            }
        }
        variant&operator=(variant&& that)noexcept(all_nothrow_moveC&&all_nothrow_moveA){
            if(idx==that.idx){
                constexpr void(*const erased[])(void *,void *)={&detail::erase_moveA<Ts>::erased...};
                erased[idx](ptr(),that.ptr());
            }else{
                destruct();idx=-1;construct_impl(std::move(that));idx=that.idx;
            }
        }
        template<size_t i>constexpr to_type<i>&get(){
            if (idx != i)throw bad_variant_access("Unexpected index");
            return *(to_type<i>*)ptr();
        }
        template<size_t i>constexpr to_type<i>&&get()&&{
            if (idx != i)throw bad_variant_access("Unexpected index");
            return std::move(*(to_type<i>*)ptr());
        }
        template<size_t i>constexpr const to_type<i>&get()const{
            if (idx != i)throw bad_variant_access("Unexpected index");
            return *(const to_type<i>*)ptr();
        }
        template<size_t i>constexpr const to_type<i>&&get()const&&{
            if (idx != i)throw bad_variant_access("Unexpected index");
            return std::move(*(const to_type<i>*)ptr());
        }
        template<size_t i>constexpr inline to_type<i>*get_if() noexcept{
            if (idx == i)return (to_type<i>*)ptr();
            return nullptr;
        }
        template<size_t i>constexpr inline const to_type<i>*get_if()const noexcept{
            if (idx == i)return (const to_type<i>*)ptr();
            return nullptr;
        }
        constexpr size_t index() const noexcept{return idx;}
    };
    namespace detail{
        template<class T,bool trivial>struct ConditionalDtor;
        template<class T>struct ConditionalDtor<T,true>:T{~ConditionalDtor()=default;};
        template<class T>struct ConditionalDtor<T,false>:T{~ConditionalDtor(){this->destruct();}};
        template<class F,class T>struct bound_func{
            F&& func;T&& first;
            template<class...Args>decltype(auto)operator()(Args&&...args){
                return invoke(std::forward<F>(func),std::forward<T>(first),std::forward<Args>(args)...);
            }
        };
        template<class F,class T,class...Rest>struct erase_visit{
            static decltype(auto)cref(F&& visitor,const void*first,Rest&&...rest){
                return visit(bound_func<F,const T&>{std::forward<F>(visitor),*(const T*)first},std::forward<Rest>(rest)...);
            }
            static decltype(auto)lref(F&& visitor,void*first,Rest&&...rest){
                return visit(bound_func<F,T&>{std::forward<F>(visitor),*(T*)first},std::forward<Rest>(rest)...);
            }
            static decltype(auto)rref(F&& visitor,void*first,Rest&&...rest){
                return visit(bound_func<F,T&&>{std::forward<F>(visitor),std::move(*(T*)first)},std::forward<Rest>(rest)...);
            }
        };
    } // namespace detail
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,const variant<Ts...>&first,Rest&&...rest){
        typedef typename variant<Ts...>::template to_type<0>First;
        typedef decltype(&detail::erase_visit<F,First,Rest...>::cref) func_t;
        constexpr const func_t erased[]={&detail::erase_visit<F,Ts,Rest...>::cref...};
        return erased[first.idx](visitor,first.ptr(),rest...);
    }
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,variant<Ts...>&first,Rest&&...rest){
        typedef typename variant<Ts...>::template to_type<0>First;
        typedef decltype(&detail::erase_visit<F,First,Rest...>::lref) func_t;
        constexpr const func_t erased[]={&detail::erase_visit<F,Ts,Rest...>::lref...};
        return erased[first.idx](visitor,first.ptr(),rest...);
    }
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,variant<Ts...>&&first,Rest&&...rest){
        typedef typename variant<Ts...>::template to_type<0>First;
        typedef decltype(&detail::erase_visit<F,First,Rest...>::rref) func_t;
        constexpr const func_t erased[]={&detail::erase_visit<F,Ts,Rest...>::rref...};
        return erased[first.idx](visitor,first.ptr(),rest...);
    }
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
}//namespace backports
template<class A,class  B>struct std::hash<backports::variant<A,B>>{
    using argument_type = backports::variant<A,B>;
    using result_type = size_t;
    size_t operator()(const backports::variant<A,B>& t) const noexcept
        ((backports::is_nothrow_invocable_v<std::hash<std::decay_t<A>>,const A&>&&backports::is_nothrow_invocable_v<std::hash<std::decay_t<B>>,const A&>)){
        if(t.index()==0){return std::hash<size_t>{}(0)+std::hash<std::decay_t<A>>{}(t.get_A());}
        if(t.index()==1){return std::hash<size_t>{}(1)+std::hash<std::decay_t<B>>{}(t.get_B());}
        return std::hash<size_t>{}(-1);
    }
};
#endif //VARIANT_HPP
