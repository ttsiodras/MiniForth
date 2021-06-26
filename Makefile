TARGET:=myforth

# CXX=clang++
CXXFLAGS+=-Wall -Wextra -fpermissive -ffunction-sections -fdata-sections -Wl,--gc-sections
GDB?=gdb

all:	${TARGET}-small

gdb:	${TARGET}-debug
	${GDB} $<

${TARGET}-small: ${TARGET}.cpp
	${CXX} -Os ${CXXFLAGS} -o $@ $<
	strip $@

${TARGET}-debug: ${TARGET}.cpp
	${CXX} -g -o $@ $<

clean:
	rm -f ${TARGET}-small ${TARGET}-debug
