#include "functional.hpp"
#include "optional.hpp"
#include "string_view.hpp"
#include "utility.hpp"
#include "variant.hpp"
#include "tuple.hpp"
#include <cstdio>
struct Foo{
    long operator()(int x)noexcept{return x+1;}
    long operator()(long x)noexcept{return x+1;}
};
Foo foo;
long(*bup)(Foo&,backports::variant<int,long>&)=&backports::visit<Foo&,backports::variant<int,long>&>;

template<class T>void printType(){puts(__PRETTY_FUNCTION__);}
int main(int argc,char**){
    backports::variant<int,long>bar;
    if(argc){bar.emplace<0>(1);}else{bar.emplace<1>(3);}
    backports::optional<long>baz=(*bup)(foo,bar);
    return backports::invoke(foo,baz.value());
}