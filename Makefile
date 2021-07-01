# Read the PORT variable
include config.mk

all:	x86

arduino:
	$(MAKE) -C src

arduino-sim:
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

extract-forth-code:
	@cat README.md                            \
	    | grep '^    '                        \
	    | grep -v OK                          \
	    | sed 's,^    ,,'

test-address-sanitizer:
	$(MAKE) -C src_x86
	@$(MAKE) extract-forth-code               \
	    | grep -v '^make'                     \
	    | ./src_x86/x86_forth 2>&1            \
	    | ( ! grep --color=always '\[x\].*' )
	@echo "[-] Test PASSED."

test-valgrind:
	$(MAKE) -C src_x86 valgrind
	@$(MAKE) extract-forth-code               \
	    | grep -v '^make'                     \
	    | valgrind ./src_x86/x86_forth
	@echo "[-] Test PASSED."

test-arduino:
	cd testing/ ; ./test_forth.py -p ${PORT}

test:
	$(MAKE) test-address-sanitizer
