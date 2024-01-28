all: Union.o test.o asan.o
	size -t Union.o test.o asan.o

CFLAGS = -Wall -Wpedantic -Wextra -Werror -Wno-maybe-uninitialized -std=c++14

Union.o: Union.cpp Union.hpp
	g++ $(CFLAGS) -O3 -s -c Union.cpp -o $@

test.o: Union.cpp Union.hpp
	g++ $(CFLAGS) -Os -g -c Union.cpp -o $@

asan.o: Union.cpp Union.hpp
	g++ $(CFLAGS) -Os -g -fsanitize=address -c Union.cpp -o $@

test.out: test.cpp Union.hpp
	g++ $(CFLAGS) -Os -g -c test.cpp -o $@

asan.out: test.cpp Union.hpp
	g++ $(CFLAGS) -Os -g -fsanitize=address test.cpp -o $@
