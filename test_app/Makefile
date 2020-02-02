#****************************************************************************
#*
#*  (C) 2020 Andrii Bilynskyi <andriy.bilynskyy@gmail.com>
#*
#*  This code is licensed under the MIT.
#*
#****************************************************************************


TOOLCHAIN_PATH := /opt/gcc-arm-none-eabi-9-2019-q4-major


CC     := ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc
CXX    := ${TOOLCHAIN_PATH}/bin/arm-none-eabi-g++
AS     := ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc
OBJCPY := ${TOOLCHAIN_PATH}/bin/arm-none-eabi-objcopy
SIZE   := ${TOOLCHAIN_PATH}/bin/arm-none-eabi-size
GDB    := ${TOOLCHAIN_PATH}/bin/arm-none-eabi-gdb


PROJECT ?= stm32-app


PROJECT_CSRC := \
  ${wildcard ../stm/*.c} \
  ${wildcard ../stm/*.s} \
  ${wildcard ../stm/STM32F10x_StdPeriph_Driver/src/*.c} \
  ${wildcard *.c} \

INCLUDE_PATH := \
  ../cm \
  ../stm \
  ../stm/STM32F10x_StdPeriph_Driver/inc \
  ./ \

LDLIBS := \
  stdc++ \

LD_SCRIPT := stm32_app.ld

OBJECTS := ${addsuffix .o, ${PROJECT_CSRC}}

CFLAGS  := -Wall -std=c99 -fdata-sections -ffunction-sections
CFLAGS  += -mlittle-endian -mthumb -mcpu=cortex-m3
CFLAGS  += -DSTM32F10X_MD -DHSE_VALUE=8000000u -DUSE_STDPERIPH_DRIVER

ASFLAGS  := -Wall -std=c99
ASFLAGS  += -mlittle-endian -mthumb -mcpu=cortex-m3
ASFLAGS  += -DSTM32F10X_MD -DHSE_VALUE=8000000u -DUSE_STDPERIPH_DRIVER

CXXFLAGS  := -Wall -fno-exceptions -fdata-sections -ffunction-sections
CXXFLAGS  += -mlittle-endian -mthumb -mcpu=cortex-m3
CXXFLAGS  += -DSTM32F10X_MD -DHSE_VALUE=8000000u -DUSE_STDPERIPH_DRIVER

LDFLAGS := -T${LD_SCRIPT}
LDFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Xlinker -Map=${PROJECT}.map
LDFLAGS += ${addprefix -l, ${LDLIBS}}


CFLAGS  += -g0 -O3
ASFLAGS += -g0 -O3
CXXFLAGS += -g0 -O3
LDFLAGS += --specs=nosys.specs


CFLAGS  += ${addprefix -I, ${INCLUDE_PATH}}
CXXFLAGS  += ${addprefix -I, ${INCLUDE_PATH}}

.PHONY: all build clean help

all: ${PROJECT}.elf

build: ${PROJECT}.elf

${PROJECT}.elf: ${OBJECTS}
	$(info $(BUILD) BUILD)
	${CC} $^ -o $@ ${LDFLAGS}
	${OBJCPY} -O ihex $@ ${PROJECT}.hex
	${SIZE} --format=berkeley $@

%.c.o: %.c
	${CC} ${CFLAGS} -MD -c $< -o $@

%.s.o: %.s
	${CC} ${ASFLAGS} -MD -c $< -o $@

%.cpp.o: %.cpp
	${CXX} ${CXXFLAGS} -MD -c $< -o $@

clean:
	rm -f ${OBJECTS} $(OBJECTS:.o=.d) ${PROJECT}.elf ${PROJECT}.hex ${PROJECT}.map

help:
	$(info Help)
	$(info ---------------------------------------------------------------------)
	$(info make help                     - this help text)
	$(info make build                    - create binary files (.elf, .hex))
	$(info make all                      - create binary files (.elf, .hex). same as build)
	$(info make clean                    - remove generated files)
	$(info ---------------------------------------------------------------------)

-include $(OBJECTS:.o=.d)
