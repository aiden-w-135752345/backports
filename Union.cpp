#include "Union.hpp"
#include <memory>

//int bup(){return visit([](int foo){return (int)foo;},Union::Union<int>{});}
Union::Union<unsigned int,char> foo;
Union::Union<unsigned int,std::shared_ptr<short>> bar;
Union::Union<unsigned int,std::unique_ptr<long>> baz;