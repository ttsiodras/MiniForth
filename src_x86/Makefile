CFLAGS:=-I. -I ../src -D __NATIVE_BUILD__

all:
	g++ -g ${CFLAGS}  -o x86_forth ../src/*.cpp myforth.cpp -fsanitize=address

valgrind:
	g++ -g ${CFLAGS}  -o x86_forth ../src/*.cpp myforth.cpp
