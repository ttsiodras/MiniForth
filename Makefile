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

test:	${TARGET}-small
	@echo "[-] Testing..."
	@cat README.md                          \
	    | grep '^    '                      \
	    | grep -v OK                        \
	    | sed 's,^    ,,'                   \
	    | ./${TARGET}-small 2>&1 >/dev/null \
	    | grep '\[x\]' ;                    \
	if [ $$? -eq 0 ] ; then                 \
	    echo "[x] Failed..." ;              \
	    exit 1 ;                            \
	else                                    \
	    echo "[-] All good!" ;              \
	    exit 0 ;                            \
       	fi

clean:
	rm -f ${TARGET}-small ${TARGET}-debug

.PHONY: clean test
