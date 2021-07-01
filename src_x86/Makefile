CFLAGS:=-I. -I ../src               \
	-D PROGMEM=""               \
	-D __FlashStringHelper=char \
	-D strcasecmp_P=strcasecmp

all:
	g++ -g ${CFLAGS}  -o x86_forth ../src/*.cpp myforth.cpp -fsanitize=address

valgrind:
	g++ -g ${CFLAGS}  -o x86_forth ../src/*.cpp myforth.cpp
