CFLAGS = -std=c++14 -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color
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
HEADERS += $(wildcard src/ryu/*) include/detail/inline_variables.hpp 
HEADERS += include/detail/invoke_traits.hpp include/detail/special_members.hpp

all: test.out backports.a
clean:
	rm test.out backports.a build/*
MY_OBJECTS=stdexcept from_chars to_chars_tables to_chars
RYU_OBJECTS=generic_128 d2s d2s_full_table d2fixed f2s d2fixed_full_table
backports.a: $(foreach obj,$(MY_OBJECTS) $(RYU_OBJECTS),build/$(obj).o)
	ar rcu $@ $^
test.out: test.cpp $(foreach obj,$(MY_OBJECTS),src/$(obj).cpp) $(foreach obj,$(RYU_OBJECTS),build/$(obj).o) $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Og -g test.cpp $(foreach obj,$(MY_OBJECTS),src/$(obj).cpp) $(foreach obj,$(RYU_OBJECTS),build/$(obj).o) -fsanitize=address -Wno-vla-extension -Wno-unused-function
$(foreach obj,$(RYU_OBJECTS),build/$(obj).o): build/%.o: src/ryu/%.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $<
build/stdexcept.o: src/stdexcept.cpp  $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $< -Weverything -Wno-c++98-compat-pedantic -Wc++14-compat-pedantic
build/from_chars.o: src/from_chars.cpp  $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $< -Weverything -Wno-c++98-compat-pedantic -Wc++14-compat-pedantic -Wno-documentation-unknown-command -Wno-float-equal -Wno-padded -Wno-unsafe-buffer-usage -Wno-unused-template
build/to_chars_tables.o: src/to_chars_tables.cpp  $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $< -Weverything -Wno-c++98-compat-pedantic -Wc++14-compat-pedantic
build/to_chars.o: src/to_chars.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -o $@ -Os -DNDEBUG -c $< -Weverything -Wno-c++98-compat-pedantic -Wc++14-compat-pedantic -Wno-format-nonliteral -Wno-padded -Wno-unsafe-buffer-usage -Wno-unused-function -Wno-unused-template
# -Wno-vla-extension