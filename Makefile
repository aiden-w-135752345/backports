CFLAGS = -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color -std=c++14 -Wno-noexcept-type
HEADERS = functional.hpp optional.hpp
HEADERS += string_view.hpp type_traits.hpp utility.hpp
HEADERS += variant.hpp tuple.hpp
HEADERS := $(foreach header,$(HEADERS),include/$(header))
all: test.out $(HEADERS)
test.out: test.cpp $(HEADERS)
	g++ $(CFLAGS) -o $@ -O2 -g $<
clean:
	test.out