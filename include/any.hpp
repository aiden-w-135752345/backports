#ifndef ANY_HPP
#define ANY_HPP
#include <typeinfo>
#include <algorithm>
#include "utility.hpp"
#include "type_traits.hpp"
#include <typeinfo>
#include "cstddef.hpp"
#include <any>
namespace backports{
    class bad_any_cast : public std::bad_cast {
        public:
        virtual const char* what() const noexcept { return "bad any_cast"; }
    };
    class any;
    namespace _any{template<class T>T* cast(const any*,std::true_type);}
    class any {
        template<class T>friend T* _any::cast(const any*,std::true_type);
        enum class AnyOp{destroy,clone,xfer,type};
        union OpArg{
            any*dest;const std::type_info**info;
            OpArg(){}OpArg(any*d):dest(d){}
            OpArg(const std::type_info**i):info(i){}
        };
        
        typedef void (any::*Manage)(AnyOp,OpArg);
        template<class T>void manage_internal(AnyOp op,OpArg arg){switch(op){
            case AnyOp::destroy:static_cast<T*>(buf)->~T();break;
            case AnyOp::clone: ::new (arg.dest->buf) T(*static_cast<const T*>(buf));break;
            case AnyOp::xfer:
                ::new (arg.dest->buf) T(std::move(*static_cast<T*>(buf)));
                static_cast<T*>(buf)->~T();
                break;
            case AnyOp::type:*arg.info=&typeid(T);break;
        }}
        template<class T>void manage_external(AnyOp op,OpArg arg){switch(op){
            case AnyOp::destroy:delete static_cast<T*>(ptr);break;
            case AnyOp::clone:arg.dest->ptr=::new T(*static_cast<const T*>(ptr));break;
            case AnyOp::xfer:arg.dest->ptr=ptr;ptr=nullptr;break;
            case AnyOp::type:*arg.info=&typeid(T);break;
        }}
        template<class...Args>using Create=void (any::*)(Args&&...);
        template<class T,class...Args>void create_internal(Args&&... args){
            ::new (buf) T(std::forward<Args>(args)...);manage=&manage_internal<T>;
        }
        template<class T,class...Args>void create_external(Args&&... args){
            ptr=::new T(std::forward<Args>(args)...);manage=&manage_external<T>;
        }
        Manage manage;
        union{
            void* ptr;
            alignas(alignof(void*))byte buf[sizeof(void*)];
        };
        template<class T>
        using decay_if_not_any=std::enable_if_t<is_copy_constructible_v<std::decay_t<T>>&&!is_same_v<std::decay_t<T>,any>, std::decay_t<T>>;
        template<class T,bool Internal=is_nothrow_move_constructible_v<T>&&(sizeof(T) <= sizeof(void*))&&(alignof(T) <= alignof(void*))>struct Manager;
        template<class T>struct Manager<T,true>{
            constexpr const static Manage manage=&manage_internal<T>;
            template<class...Args>constexpr const static Create<Args...>create=&create_internal<T>;
            constexpr const static auto access=&any::buf;
        };
        template<class T>struct Manager<T,false>{
            constexpr const static Manage manage=&manage_external<T>;
            template<class...Args>constexpr const static Create<Args...>create=&create_external<T>;
            constexpr const static auto access=&any::ptr;
        };
        template<class T>struct not_in_place{using type=T;};
        template<class T>struct not_in_place<in_place_type_t<T>>{};
        template<class T>using not_in_place_t=typename not_in_place<T>::type;
        template<class R,class T,class... Args>using any_constructible=std::enable_if_t<is_copy_constructible_v<T>&&is_constructible_v<T, Args...>,R>;
        template<class T, class... Args>using any_constructible_t= any_constructible<std::decay_t<T>, std::decay_t<T>, Args...>;
        template<class T, class... Args>using emplace_t=any_constructible<std::decay_t<T>&, std::decay_t<T>, Args...>;
    public:
        constexpr any() noexcept:manage(nullptr),ptr(nullptr){};
        any(const any& other):manage(other.manage){
            if(manage){(const_cast<any&>(other).*manage)(AnyOp::clone,{this});}
        };
        any(any&& other) noexcept:manage(other.manage){
            if(manage){(other.*manage)(AnyOp::xfer,{this});other.manage=nullptr;}
        };
        template <class T,class V = not_in_place_t<decay_if_not_any<T>>>any(T&& value)
            {(this->*Manager<V>::create)(std::forward<T>(value));}
        template <class T,class...Args,class V=any_constructible_t<T,Args&&...>>
        explicit any(in_place_type_t<T>, Args&&... args)
            { (this->*Manager<V>::create)(std::forward<Args>(args)...); }
        template <class T,class U,class...Args,class V=any_constructible_t<T,std::initializer_list<U>&,Args&&...>>
        explicit any(in_place_type_t<T>, std::initializer_list<U> il, Args&&... args)
            { (this->*Manager<V>::create)(il, std::forward<Args>(args)...); }
        ~any(){reset();}
        any& operator=(const any& rhs){return *this=any(rhs);}
        any& operator=(any&& rhs) noexcept{
            if (!rhs.has_value()) reset();
            else if (this != &rhs) {
                reset();
                (rhs.*rhs.manage)(AnyOp::xfer,{this});
            }
            return *this;
        }
        template<class T> std::enable_if_t<is_copy_constructible_v<decay_if_not_any<T>>, any&>
        operator=(T&& rhs){return *this = any(std::forward<T>(rhs));}
        template <class T, class... Args>emplace_t<T, Args...>emplace(Args&&... args){
            using Mgr = Manager<std::decay_t<T>>;
            reset();
            (this->*Mgr::create)(std::forward<Args>(args)...);
            return *static_cast<T*>(this->*Mgr::access);
        }
        template <class T, class U, class... Args>emplace_t<T, std::initializer_list<U>&, Args&&...>
        emplace(std::initializer_list<U> il, Args&&... args){
            using Mgr = Manager<std::decay_t<T>>;
            reset();
            (this->*Mgr::create)(il,std::forward<Args>(args)...);
            return *static_cast<T*>(this->*Mgr::access);
        }
        void reset() noexcept{
            if(manage){(this->*manage)(AnyOp::destroy,{});manage=nullptr;}
        }
        void swap(any& rhs) noexcept{
            any&lhs=*this;
            if (lhs.manage&&rhs.manage){
                if (&lhs == &rhs)return;
                any tmp;
                (rhs.*rhs.manage)(AnyOp::xfer,{&tmp});tmp.manage=rhs.manage;
                (lhs.*lhs.manage)(AnyOp::xfer,{&rhs});rhs.manage=lhs.manage;
                (tmp.*tmp.manage)(AnyOp::xfer,{&lhs});lhs.manage=tmp.manage;
                tmp.manage=nullptr;
            } else if(lhs.manage) {
                (lhs.*lhs.manage)(AnyOp::xfer,{&rhs});
                rhs.manage=lhs.manage;lhs.manage=nullptr;
            } else if(rhs.manage) {
                (rhs.*rhs.manage)(AnyOp::xfer,{&lhs});
                lhs.manage=rhs.manage;rhs.manage=nullptr;
            }
        }
        bool has_value() const noexcept{return manage!=nullptr;}
        const std::type_info& type() const noexcept{
            const std::type_info*type=&typeid(void);
            if(manage){(const_cast<any*>(this)->*manage)(AnyOp::type,{&type});}
            return *type;
        }
    };
    inline void swap(any& x, any& y) noexcept { x.swap(y); }


    template <typename T, typename... Args>inline
    std::enable_if_t<is_constructible_v<any, in_place_type_t<T>, Args...>, any>
    make_any(Args&&... args) { return any(in_place_type<T>, std::forward<Args>(args)...); }
    template <typename T, typename U, typename... Args>inline
    std::enable_if_t<is_constructible_v<any, in_place_type_t<T>,std::initializer_list<U>&, Args...>, any>
    make_any(std::initializer_list<U> il, Args&&... args)
        { return any(in_place_type<T>, il, std::forward<Args>(args)...); }

    namespace _any{
        template<class T>using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
        template<class T>constexpr static bool is_valid_cast=is_reference_v<T>||is_copy_constructible_v<T>;
        template<class T>T* cast(const any* value,std::true_type){
            using U = std::remove_cv_t<T>;
            if (value->manage == &any::Manager<U>::manage|| value->type() == typeid(T)){
                return static_cast<T*>(value->*any::Manager<U>::access);
            }
            return nullptr;
        }
        template<class T>T* cast(const any*,std::false_type){return nullptr;}
        template<class T>T* cast(const any* value){
            using U = std::remove_cv_t<T>;
            return cast<T>(value,bool_constant<is_object_v<T>&&is_same_v<std::decay_t<U>, U>&&is_copy_constructible_v<U>>{});
        }
    };
    template<class T>inline T any_cast(const any& value){
        using U = _any::remove_cvref_t<T>;
        static_assert(_any::is_valid_cast<T>,"Template argument must be a reference or CopyConstructible type");
        static_assert(is_constructible_v<T, const U&>,"Template argument must be constructible from a const value.");
        auto p = any_cast<U>(&value);
        if (p)return static_cast<T>(*p);
        throw bad_any_cast{};
    }
    template<class T>inline T any_cast(any& value){
        using U = _any::remove_cvref_t<T>;
        static_assert(_any::is_valid_cast<T>,
        "Template argument must be a reference or CopyConstructible type");
        static_assert(is_constructible_v<T, U&>,
        "Template argument must be constructible from an lvalue.");
        auto p = any_cast<U>(&value);
        if (p)return static_cast<T>(*p);
        throw bad_any_cast{};
    }
    template<class T>inline T any_cast(any&& value){
        using U = _any::remove_cvref_t<T>;
        static_assert(_any::is_valid_cast<T>,
        "Template argument must be a reference or CopyConstructible type");
        static_assert(is_constructible_v<T, U>,
        "Template argument must be constructible from an rvalue.");
        auto p = any_cast<U>(&value);
        if (p)return static_cast<T>(std::move(*p));
        throw bad_any_cast{};
    }
    template<class T>inline const T* any_cast(const any* value) noexcept{
        if (value)return _any::cast<T>(value);
        return nullptr;
    }
    template<class T>inline T* any_cast(any* value) noexcept{
        if (value)return _any::cast<T>(value);
        return nullptr;
    }




}// namespace backports
#endif // ANY_HPP