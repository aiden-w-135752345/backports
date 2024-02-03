#include "functional.hpp"
#include <cstdio>
int foo(int x)noexcept{return x;}
template<class T>void printType(){
    puts(__PRETTY_FUNCTION__);
}
int main(int,char**){
    return backports::invoke<decltype(&foo),int>(&foo,13);
}