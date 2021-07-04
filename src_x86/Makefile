CFLAGS:=-I. -I ../src               \
	-D PROGMEM=""               \
	-D __FlashStringHelper=char \
	-D strcasecmp_P=strcasecmp  \
	-D PGM_P="const char *"

all:
	g++ -g ${CFLAGS}  -o x86_forth ../src/*.cpp myforth.cpp -fsanitize=address

valgrind:
	g++ -g ${CFLAGS}  -o x86_forth ../src/*.cpp myforth.cpp
