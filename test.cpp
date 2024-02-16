#include "include/functional.hpp"
#include "include/optional.hpp"
#include "include/string_view.hpp"
#include "include/tuple.hpp"
#include "include/type_traits.hpp"
#include "include/utility.hpp"
#include "include/variant.hpp"
#include <iostream>
struct Foo{
    long operator()(int x)noexcept{return x+1;}
    long operator()(long x)noexcept{return x+1;}
};
Foo foo;
template<class T>void printType(){
    backports::string_view sv{__PRETTY_FUNCTION__};
    std::cout<<sv<<'\n';
}
int main(int argc,char**){
    backports::variant<int,long>bar;
    if(argc){bar.emplace<0>(1);}else{bar.emplace<1>(3);}
    backports::optional<long>baz=backports::visit(foo,bar);
    std::cout<<backports::invoke(foo,baz.value())<<'\n';
    std::cout<<backports::tuple_size_v<decltype(std::make_tuple(1,2,3))> <<'\n';
    printType<backports::in_place_type_t<backports::bool_constant<true>>>();
    return 0;
}