MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(patsubst %/,%,$(dir ${MKFILE_PATH}))
BUILD_DIR := ${CURRENT_DIR}/tmp

# Read the PORT variable
include ../config.mk

SRC:=myforth.ino
ELF:=tmp/${SRC}.elf

BASE:=/usr/share/arduino
USER_BASE:=$(HOME)/.arduino15
USER_LIBS:=$(HOME)/Arduino/libraries
BOARD:=arduino:avr:uno

HARDWARE:=-hardware ${BASE}/hardware -hardware ${USER_BASE}/packages 
TOOLS:=-tools ${BASE}/tools-builder -tools ${USER_BASE}/packages
LIBRARIES=-built-in-libraries ${BASE}/lib
LIBRARIES+=-libraries ${USER_LIBS}  # Where U8g2 comes from
WARNINGS:=-warnings all -logger human

ARDUINO_BUILDER_OPTS=${HARDWARE} ${TOOLS} ${LIBRARIES}
ARDUINO_BUILDER_OPTS+=-fqbn=${BOARD} ${WARNINGS}
ARDUINO_BUILDER_OPTS+=-verbose -build-path ${BUILD_DIR} 

all:
	@mkdir -p ${BUILD_DIR}
	arduino-builder -compile ${ARDUINO_BUILDER_OPTS} ${SRC} 2>&1 | tee build.log
	@grep -i error build.log || avr-size tmp/*.ino.elf
	@grep -i ' error: ' build.log ;            \
	if [ $$? -eq 0 ] ; then                   \
	    echo "[x] Failed..." ;                \
	    exit 1 ;                              \
	else                                      \
	    exit 0 ;                              \
	fi

tags:
	ctags -R . ${USER_LIBS} ${USER_BASE}

clean:
	rm -rf ${BUILD_DIR} build.log

sim:	all
	$(MAKE) -C ../simavr/examples/board_simduino/
	./simduino

x86:
	g++ -g -D __FlashStringHelper=char -I. -o x86 *.cpp myforth.ino

upload:	all
	killall picocom || exit 0
	killall minicom || exit 0
	HEX="$$(echo tmp/*.ino.hex)" ; avrdude -C${USER_BASE}/packages/arduino/tools/avrdude/6.3.0-arduino9/etc/avrdude.conf -v -patmega328p -carduino -P${PORT} -b115200 -D -Uflash:w:$${HEX}:i
	@echo -e "Issue '\e[1mmake terminal\e[0m' to communicate over ${PORT}"

terminal:
	picocom -c -b 115200 --imap lfcrlf ${PORT}
