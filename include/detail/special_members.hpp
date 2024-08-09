#include <utility>
#ifndef SPECIAL_MEMBERS_HPP
#define SPECIAL_MEMBERS_HPP
namespace backports{
    namespace _special_members{
        template<class T,bool>struct destruct:T{
            using T::T;
            ~destruct(){this->T::destruct();}
        };
        template<class T>struct destruct<T,true>:T{using T::T;};
        template<class T,bool,bool>struct moveAssign:T{
            using T::T;
            moveAssign()                             = default;
            moveAssign(const moveAssign&)            = default;
            moveAssign(moveAssign&&)                 = default;
            moveAssign& operator=(const moveAssign&) = default;
            moveAssign& operator=(moveAssign&&)      = delete;
        };
        template<class T>struct moveAssign<T,true,false>:T{
            using T::T;
            moveAssign()                             = default;
            moveAssign(const moveAssign&)            = default;
            moveAssign(moveAssign&&)                 = default;
            moveAssign& operator=(const moveAssign&) = default;
            moveAssign& operator=(moveAssign&&that)noexcept(noexcept(std::declval<T>().assign(std::move(that)))){this->assign(std::move(that));return *this;}
        };
        template<class T>struct moveAssign<T,true,true>:T{using T::T;};
        template<class T,bool,bool>struct copyAssign:T{
            using T::T;
            copyAssign()                             = default;
            copyAssign(const copyAssign&)            = default;
            copyAssign(copyAssign&&)                 = default;
            copyAssign& operator=(const copyAssign&) = delete;
            copyAssign& operator=(copyAssign&&)      = default;
        };
        template<class T>struct copyAssign<T,true,false>:T{
            using T::T;
            copyAssign()                             = default;
            copyAssign(const copyAssign&)            = default;
            copyAssign(copyAssign&&)                 = default;
            copyAssign& operator=(const copyAssign&that)noexcept(noexcept(std::declval<T>().assign(that))){this->assign(that);return *this;}
            copyAssign& operator=(copyAssign&&)      = default;
        };
        template<class T>struct copyAssign<T,true,true>:T{using T::T;};
        template<class T,bool,bool>struct moveConstruct:T{
            using T::T;
            moveConstruct()                                = default;
            moveConstruct(const moveConstruct&)            = default;
            moveConstruct(moveConstruct&&that)             = delete;
            moveConstruct& operator=(const moveConstruct&) = default;
            moveConstruct& operator=(moveConstruct&&)      = default;
        };
        template<class T>struct moveConstruct<T,true,false>:T{
            using T::T;
            moveConstruct()                                = default;
            moveConstruct(const moveConstruct&)            = default;
            moveConstruct(moveConstruct&&that)noexcept(noexcept(std::declval<T>().construct(that))){this->construct(std::move(that));}
            moveConstruct& operator=(const moveConstruct&) = default;
            moveConstruct& operator=(moveConstruct&&)      = default;
        };
        template<class T>struct moveConstruct<T,true,true>:T{using T::T;};
        template<class T,bool,bool>struct copyConstruct:T{
            using T::T;
            copyConstruct()                                = default;
            copyConstruct(const copyConstruct&that)        = delete;
            copyConstruct(copyConstruct&&)                 = default;
            copyConstruct& operator=(const copyConstruct&) = default;
            copyConstruct& operator=(copyConstruct&&)      = default;
        };
        template<class T>struct copyConstruct<T,true,false>:T{
            using T::T;
            copyConstruct()                                = default;
            copyConstruct(const copyConstruct&that)noexcept(noexcept(std::declval<T>().construct(that))){this->construct(that);}
            copyConstruct(copyConstruct&&)                 = default;
            copyConstruct& operator=(const copyConstruct&) = default;
            copyConstruct& operator=(copyConstruct&&)      = default;
        };
        template<class T>struct copyConstruct<T,true,true>:T{using T::T;};
    }
    namespace detail{
        template<class T,bool cc,bool tcc,bool mc,bool tmc,bool ca,bool tca,bool ma,bool tma,bool td>
            using special_members=_special_members::destruct<_special_members::moveAssign<_special_members::copyAssign<
            _special_members::moveConstruct<_special_members::copyConstruct<T,cc,tcc>,mc,tmc>,ca,tca>,ma,tma>,td>;
    }
}
#endif //SPECIAL_MEMBERS_HPP
