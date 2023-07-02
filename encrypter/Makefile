#****************************************************************************
#*
#*  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
#*
#*  This code is licensed under the MIT.
#*
#****************************************************************************


CC     := gcc
LD     := ld

PROJECT ?= hex-encrypter
HEXFILE ?= ../test_app/stm32-app.hex

PROJECT_CSRC := \
  ${wildcard *.c} \
  ${wildcard ../tiny-AES-c/*.c} \
  ${wildcard ../hex_parser/*.c} \
  ${wildcard ../keys/*.bin} \

INCLUDE_PATH := \
  ./ \
  ../tiny-AES-c/ \
  ../hex_parser/ \

LDLIBS := \

OBJECTS := ${addsuffix .O, ${PROJECT_CSRC}}

CFLAGS  := -g3 -O1 -Wall -std=gnu99 -fdata-sections -ffunction-sections

LDFLAGS := -Wl,--gc-sections -static

BINFLAGS := -r -b binary

CFLAGS  += ${addprefix -I, ${INCLUDE_PATH}}
LDFLAGS += ${addprefix -l, ${LDLIBS}}

.PHONY: all build clean run

all: ${PROJECT}

build: ${PROJECT}

${PROJECT}: ${OBJECTS}
	${CC} $^ -o $@ ${LDFLAGS}

%.c.O: %.c
	${CC} ${CFLAGS} -MD -c $< -o $@

%.bin.O: %.bin
	${LD} ${BINFLAGS} $< -o $@

%.bin: %.bin.o
	@:

clean:
	rm -f ${OBJECTS} $(OBJECTS:.O=.d) ${PROJECT}

run:
	@if [ -f "${PROJECT}" ];                                                    \
  then                                                                          \
    ./${PROJECT} $(HEXFILE);                                                    \
  else                                                                          \
    echo "Output executable file doesn't exist. Run 'make all' before!";        \
  fi

help:
	$(info Help)
	$(info ---------------------------------------------------------------------)
	$(info make help                     - this help text)
	$(info make build                    - create executable)
	$(info make all                      - create executable)
	$(info make clean                    - remove generated files)
	$(info make run                      - run executable)
	$(info ---------------------------------------------------------------------)

-include $(OBJECTS:.O=.d)
