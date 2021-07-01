# Read the PORT variable
include config.mk

all:	x86

arduino:
	$(MAKE) -C src

arduino-sim:
	$(MAKE) -C src
	$(MAKE) -C src sim

upload:
	$(MAKE) -C src upload

terminal:
	$(MAKE) -C src terminal

x86:
	$(MAKE) -C src_x86

clean:
	$(MAKE) -C src clean
	rm -f src_x86/x86_forth

test-address-sanitizer:
	$(MAKE) -C src_x86
	@echo "[-] Testing normally..."
	@cat README.md                            \
	    | grep '^    '                        \
	    | grep -v OK                          \
	    | sed 's,^    ,,'                     \
	    | ./src_x86/x86_forth 2>&1            \
	    | tee /dev/stderr | grep '\[x\]' ;    \
	if [ $$? -eq 0 ] ; then                   \
	    echo "[x] Failed..." ;                \
	    exit 1 ;                              \
	else                                      \
	    exit 0 ;                              \
       	fi

test-valgrind:
	$(MAKE) -C src_x86 valgrind
	@echo "[-] Testing with valgrind..."
	cat README.md                             \
	    | grep '^    '                        \
	    | grep -v OK                          \
	    | sed 's,^    ,,'                     \
	    | valgrind ./src_x86/x86_forth

test-arduino:
	cd testing/ ; ./test_forth.py -p ${PORT}

test:
	$(MAKE) test-address-sanitizer
