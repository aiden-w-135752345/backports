#include <type_traits>
#include <utility>
#include <cstddef>
#include <exception>
#include <bits/functional_hash.h>
#include <bits/invoke.h>
class bad_union_access : public std::exception{
public:
    bad_union_access() noexcept : reason("Unknown reason") { }
    const char* what() const noexcept override{ return reason; }
  private:
    bad_union_access(const char* reason) : reason(reason) { }
    const char* reason;
    friend void throw_bad_union_access(const char* what);
};
inline void throw_bad_union_access(const char* what){ _GLIBCXX_THROW_OR_ABORT(bad_union_access(what)); }


namespace Union {
    template<class...Ts> class Union;
    template<class F>constexpr decltype(auto)visit(F&& visitor){return std::__invoke(std::forward<F>(visitor));}
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,const Union<Ts...>&first,Rest&&...rest);
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,Union<Ts...>&first,Rest&&...rest);
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,Union<Ts...>&&first,Rest&&...rest);
    template<size_t i>struct in_place_index{constexpr static size_t value=i;};
    template<class T>struct in_place_type{using type=T;};
    namespace detail {
        template<size_t i,class,class...Ts>struct to_type_impl:to_type_impl<i-1,Ts...>{};
        template<class T,class...Ts>struct to_type_impl<0,T,Ts...>:in_place_type<T>{};
        //template<class...>struct to_index_impl:in_place_index<-1>{};
        //template<class T,class T1,class...Ts>struct to_index_impl<T,T1,Ts...>:in_place_index<to_index_impl<T,Ts...>::value+1>{};
        //template<class T,class...Ts>struct to_index_impl<T,T,Ts...>:in_place_index<0>{};
        template<size_t...>constexpr static size_t max=0;
        template<size_t x,size_t...rest>constexpr static size_t max<x,rest...> = (x>max<rest...>)?x:max<rest...>;
        template<size_t...>constexpr static size_t min=(size_t)-1;
        template<size_t x,size_t...rest>constexpr static size_t min<x,rest...> = (x<min<rest...>)?x:min<rest...>;
        template<size_t...>constexpr static bool logical_and=true;
        template<size_t x,size_t...rest>constexpr static bool logical_and<x,rest...> = x&&logical_and<rest...>;
        template<size_t...>constexpr static bool logical_or=false;
        template<size_t x,size_t...rest>constexpr static bool logical_or<x,rest...> = x||logical_or<rest...>;
        template<class...Ts>struct template_union{alignas(Ts...) unsigned char bytes[max<sizeof(Ts)...>];};
        namespace swappable{
            using std::swap;
            template<class T,typename = decltype(swap(std::declval<T&>(), std::declval<T&>()))>
                static std::true_type test_impl(int);
            template<typename>static std::false_type test_impl(...);
            template<class T>struct test:decltype(test_impl<T>(0)){};
        }
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
    template<class...Ts>class Union{
        constexpr static const bool all_nothrow_copyC=detail::logical_and<std::is_nothrow_copy_constructible<Ts>::value...>;
        constexpr static const bool all_nothrow_moveC=detail::logical_and<std::is_nothrow_move_constructible<Ts>::value...>;
        constexpr static const bool all_nothrow_copyA=detail::logical_and<std::is_nothrow_copy_assignable<Ts>::value...>;
        constexpr static const bool all_nothrow_moveA=detail::logical_and<std::is_nothrow_move_assignable<Ts>::value...>;
        constexpr static const bool all_trivial_dtor =detail::logical_and<std::is_trivially_destructible<Ts>::value...>;
        detail::template_union<Ts...> _M_u;
        unsigned char idx;
        void construct_impl(const Union&that){
            constexpr void(*const erased[])(void *, const void *)={&detail::erase_copyC<Ts>::erased...};
            erased[idx](ptr(),that.ptr());
        }
        void construct_impl(Union&& that){
            constexpr void(*const erased[])(void *,void *)={&detail::erase_moveC<Ts>::erased...};
            erased[idx](ptr(),that.ptr());
        }
        void destruct(){
            constexpr void(*const erased[])(void *)={&detail::erase_dtor<Ts>::erased...};
            erased[idx](ptr());
        }
        inline constexpr void* ptr(){return &_M_u;}
        inline constexpr const void* ptr()const{return &_M_u;}
        template<class F,class...Rest>friend constexpr decltype(auto)visit(F&& visitor,const Union&first,Rest&&...rest);
        template<class F,class...Rest>friend constexpr decltype(auto)visit(F&& visitor,Union&first,Rest&&...rest);
        template<class F,class...Rest>friend constexpr decltype(auto)visit(F&& visitor,Union&&first,Rest&&...rest);
    public:
        template<size_t i>using to_type=typename detail::to_type_impl<i,Ts...>::type;
        template<class = std::enable_if_t<std::is_default_constructible<to_type<0>>::value>>
            constexpr Union() noexcept(std::is_nothrow_default_constructible<to_type<0>>::value):Union(in_place_index<0>{}){}
        template<size_t i,class... Args,class = std::enable_if_t<std::is_constructible<to_type<i>,Args...>::value>>
            constexpr explicit Union(in_place_index<i>, Args&&... args) noexcept(std::is_nothrow_constructible<to_type<i>,Args...>::value)
                :idx(i){::new (ptr()) to_type<i>(std::forward<Args>(args)...);}
        constexpr Union(const Union&that)noexcept(all_nothrow_copyC):_M_u(),idx(that.idx){construct_impl(that);}
        constexpr Union(Union&&that)noexcept(all_nothrow_moveC):_M_u(),idx(that.idx){construct_impl(std::move(that));}
        ~Union(){destruct();};
        Union&operator=(const Union&that)noexcept(all_nothrow_copyC&&all_nothrow_copyA){
            if(idx==that.idx){
                constexpr void(*const erased[])(void *, const void *)={&detail::erase_copyA<Ts>::erased...};
                erased[idx](ptr(),that.ptr());
            }else{
                destruct();idx=-1;construct_impl(that);idx=that.idx;
            }
        }
        Union&operator=(Union&& that)noexcept(all_nothrow_moveC&&all_nothrow_moveA){
            if(idx==that.idx){
                constexpr void(*const erased[])(void *,void *)={&detail::erase_moveA<Ts>::erased...};
                erased[idx](ptr(),that.ptr());
            }else{
                destruct();idx=-1;construct_impl(std::move(that));idx=that.idx;
            }
        }
        template<size_t i>constexpr to_type<i>&get(){
            if (idx != i)throw_bad_union_access("Unexpected index");
            return *(to_type<i>*)ptr();
        }
        template<size_t i>constexpr to_type<i>&&get()&&{
            if (idx != i)throw_bad_union_access("Unexpected index");
            return std::move(*(to_type<i>*)ptr());
        }
        template<size_t i>constexpr const to_type<i>&get()const{
            if (idx != i)throw_bad_union_access("Unexpected index");
            return *(const to_type<i>*)ptr();
        }
        template<size_t i>constexpr const to_type<i>&&get()const&&{
            if (idx != i)throw_bad_union_access("Unexpected index");
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
                return std::__invoke(std::forward<F>(func),std::forward<T>(first),std::forward<Args>(args)...);
            }
        };
        //  template<class T> inline constexpr bool holds_alternative()const noexcept{return index()==to_index<T>;}
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
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,const Union<Ts...>&first,Rest&&...rest){
        typedef typename Union<Ts...>::template to_type<0>First;
        typedef decltype(&detail::erase_visit<F,First,Rest...>::cref) func_t;
        constexpr const func_t erased[]={&detail::erase_visit<F,Ts,Rest...>::cref...};
        return erased[first.idx](visitor,first.ptr(),rest...);
    }
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,Union<Ts...>&first,Rest&&...rest){
        typedef typename Union<Ts...>::template to_type<0>First;
        typedef decltype(&detail::erase_visit<F,First,Rest...>::lref) func_t;
        constexpr const func_t erased[]={&detail::erase_visit<F,Ts,Rest...>::lref...};
        return erased[first.idx](visitor,first.ptr(),rest...);
    }
    template<class F,class...Ts,class...Rest>constexpr decltype(auto)visit(F&& visitor,Union<Ts...>&&first,Rest&&...rest){
        typedef typename Union<Ts...>::template to_type<0>First;
        typedef decltype(&detail::erase_visit<F,First,Rest...>::rref) func_t;
        constexpr const func_t erased[]={&detail::erase_visit<F,Ts,Rest...>::rref...};
        return erased[first.idx](visitor,first.ptr(),rest...);
    }
#define RELATION_FUNCTION_TEMPLATE(OP) \
    template<class A,class B>constexpr bool operator OP(const Union<A,B>& lhs,const Union<A,B>& rhs) { \
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
}//namespace Union
template<class A,class  B>struct std::hash<Union::Union<A,B>>{
    using argument_type = Union::Union<A,B>;
    using result_type = size_t;
    size_t operator()(const Union::Union<A,B>& t) const noexcept((std::__is_nothrow_invocable<std::hash<std::decay_t<A>>,A>::value && std::__is_nothrow_invocable<std::hash<std::decay_t<B>>,B>::value)){
        if(t.index()==0){return std::hash<size_t>{}(0)+std::hash<A>{}(t.get_A());}
        if(t.index()==1){return std::hash<size_t>{}(1)+std::hash<B>{}(t.get_B());}
        return std::hash<size_t>{}(-1);
    }
};