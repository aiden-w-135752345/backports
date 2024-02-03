

CFLAGS = -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color -std=c++14 -Wno-noexcept-type
COMPILED_HEADERS=functional.hpp.gch optional.hpp.gch string_view.hpp.gch type_traits.hpp.gch utility.hpp.gch variant.hpp.gch

all: test.o $(COMPILED_HEADERS)
%.hpp.gch : %.hpp
	g++ $(CFLAGS) $<
test.o: test.cpp type_traits.hpp.gch
	g++ $(CFLAGS) -c -O2 $< -o $@
clean:
	rm *.gch