CFLAGS = -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color -std=c++14 -Wno-noexcept-type
#CXXFLAGS += -flto=full
CXX = clang++

HEADERS =  include/algorithm.hpp include/any.hpp
HEADERS += include/array.hpp include/charconv.hpp
HEADERS += include/chrono.hpp include/cmath.hpp
HEADERS += include/cstddef.hpp include/ctime.hpp
HEADERS += include/execution.hpp include/functional.hpp
HEADERS += include/optional.hpp include/string_view.hpp
HEADERS += include/tuple.hpp include/type_traits.hpp
HEADERS += include/utility.hpp include/variant.hpp 
HEADERS += $(wildcard src/ryu/*) src/inline_variables.hpp 
HEADERS += src/invoke_traits.hpp src/special_members.hpp

all: test.out backports.a
clean:
	rm test.out backports.a build/*
MY_OBJECTS=to_chars_tables from_chars to_chars
RYU_OBJECTS=generic_128 d2s d2s_full_table d2fixed f2s d2fixed_full_table
backports.a: $(foreach obj,$(MY_OBJECTS) $(RYU_OBJECTS),build/$(obj).o)
	ar rcu $@ $^
test.out: test.cpp $(foreach obj,$(MY_OBJECTS),src/$(obj).cpp) $(foreach obj,$(RYU_OBJECTS),build/$(obj).o) $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Og -g test.cpp $(foreach obj,$(MY_OBJECTS),src/$(obj).cpp) $(foreach obj,$(RYU_OBJECTS),build/$(obj).o) -Wno-vla -fsanitize=address
$(foreach obj,$(RYU_OBJECTS),build/$(obj).o): build/%.o: src/ryu/%.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $<

build/to_chars_tables.o build/from_chars.o: build/%.o: src/%.cpp  $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $<
build/to_chars.o: src/to_chars.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $< -Wno-vla