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

AVRDUDE_OPTS=-C${USER_BASE}/packages/arduino/tools/avrdude/6.3.0-arduino9/etc/avrdude.conf
AVRDUDE_OPTS+=-v -patmega328p -carduino -P${PORT} -b115200 -D

all:
	@mkdir -p ${BUILD_DIR}
	arduino-builder -compile ${ARDUINO_BUILDER_OPTS} ${SRC} 2>&1 | tee build.log
	@grep -i error build.log || avr-size tmp/*.ino.elf
	@! grep -i ' error: ' build.log

clean:
	rm -rf ${BUILD_DIR} build.log

upload:	all
	killall picocom || exit 0
	killall minicom || exit 0
	HEX="$$(echo tmp/*.ino.hex)" ; avrdude ${AVRDUDE_OPTS} -Uflash:w:$${HEX}:i
