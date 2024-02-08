

CFLAGS = -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color -std=c++14 -Wno-noexcept-type
COMPILED_HEADERS = functional.hpp.gch optional.hpp.gch
COMPILED_HEADERS += string_view.hpp.gch type_traits.hpp.gch utility.hpp.gch
COMPILED_HEADERS += variant.hpp.gch tuple.hpp.gch

all: test.out $(COMPILED_HEADERS)
%.hpp.gch : %.hpp
	g++ $(CFLAGS) $<
test.out: test.cpp $(COMPILED_HEADERS)
	g++ $(CFLAGS) -O2 -g $< -o $@
clean:
	rm *.gch