all:
	g++ -g -D PROGMEM="" -D __FlashStringHelper=char -I. -I../src -o x86_forth ../src/*.cpp myforth.cpp -fsanitize=address

valgrind:
	g++ -g -D PROGMEM="" -D __FlashStringHelper=char -I. -I../src -o x86_forth ../src/*.cpp myforth.cpp
