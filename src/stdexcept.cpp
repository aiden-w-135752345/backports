#include "../include/optional.hpp"
#include "../include/variant.hpp"
const char* backports::bad_optional_access::what() const noexcept{return "bad optional access";}
const char* backports::bad_variant_access::what() const noexcept{return reason;}
