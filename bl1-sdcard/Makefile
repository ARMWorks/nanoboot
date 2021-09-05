# SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
#
# Copyright (C) 2021 Jeff Kent <jeff@jkent.net>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

MAKEFLAGS				:= -j $(shell nproc)

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
NULL := 2>/dev/null
endif

###############################################################################
# Executables

PREFIX			?= arm-none-eabi-

CC				:= $(PREFIX)gcc
CXX				:= $(PREFIX)g++
CPP				:= $(PREFIX)cpp
LD				:= $(PREFIX)gcc
AR				:= $(PREFIX)ar
AS				:= $(PREFIX)as
OBJCOPY			:= $(PREFIX)objcopy
OBJDUMP			:= $(PREFIX)objdump
GDB				:= $(PREFIX)gdb
OPT				:= -Os
CSTD			?= -std=gnu11

BINARY          := bl1
LDFLAGS			:= -specs=nano.specs -specs=nosys.specs

ifeq ($(NDEBUG),1)
PROFILE			:= release
else
DEBUG			:= -ggdb3 -DDEBUG
PROFILE			:= debug
endif

LDSCRIPT		:= build/${PROFILE}/$(BINARY)_generated.ld

###############################################################################
# Source files

VPATH			:= ../common

COMMON_AS_SRCS  := ../common/sdram.S
AS_SRCS			:= startup.S

COMMON_C_SRCS	:= $(wildcard ../common/*.c)
C_SRCS			:= $(wildcard *.c)

OBJS			:= $(patsubst ../common/%.S,build/$(PROFILE)/%.o,$(COMMON_AS_SRCS)) \
				   $(patsubst ../common/%.c,build/$(PROFILE)/%.o,$(COMMON_C_SRCS)) \
				   $(patsubst %.S,build/$(PROFILE)/%.o,$(AS_SRCS)) \
				   $(patsubst %.c,build/$(PROFILE)/%.o,$(C_SRCS))

INCLUDES        := -Iinclude \
				   -I../common/include

DEFS			+= $(INCLUDES) -DBL1 -D__KERNEL__ -D__LINUX_ARM_ARCH__=7
FP_FLAGS		?= -mfloat-abi=soft
ARCH_FLAGS		:= -mthumb -mthumb-interwork -mcpu=cortex-a8 $(FP_FLAGS)

###############################################################################
# C flags

TGT_CFLAGS		+= $(OPT) $(CSTD) $(DEBUG)
TGT_CFLAGS		+= $(ARCH_FLAGS)
TGT_CFLAGS		+= -Wextra -Wimplicit-function-declaration
TGT_CFLAGS		+= -Wstrict-prototypes -Wno-unused-parameter
TGT_CFLAGS      += -Wno-implicit-fallthrough -Wno-main -Wno-shift-negative-value
TGT_CFLAGS		+= -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C++ flags

TGT_CXXFLAGS	+= $(OPT) $(CXXSTD) $(DEBUG)
TGT_CXXFLAGS	+= $(ARCH_FLAGS)
TGT_CXXFLAGS	+= -Wextra
TGT_CXXFLAGS	+= -fno-common -ffunction-sections -fdata-sections

###############################################################################
# C & C++ preprocessor common flags

TGT_CPPFLAGS	+= -MD
TGT_CPPFLAGS	+= -Wall -Wundef
TGT_CPPFLAGS	+= $(DEFS)

###############################################################################
# Linker flags

TGT_LDFLAGS		+= --static
TGT_LDFLAGS		+= -T$(LDSCRIPT)
TGT_LDFLAGS		+= $(ARCH_FLAGS) $(DEBUG)
TGT_LDFLAGS		+= -Wl,-Map=$(patsubst %.elf,%.map,$(@)) -Wl,--cref
TGT_LDFLAGS		+= -Wl,--gc-sections
ifeq ($(V),99)
TGT_LDFLAGS		+= -Wl,--print-gc-sections
endif

###############################################################################
# Used libraries

LDLIBS			+= -Wl,--start-group -lc -lgcc -Wl,--end-group

###############################################################################

.SUFFIXES: .elf .bin .list .map .images
.SECONDEXPANSION:
.SECONDARY:

all: elf bin

elf: build/$(PROFILE)/$(BINARY).elf
bin: build/$(PROFILE)/$(BINARY).bin
list: build/$(PROFILE)/$(BINARY).list
GENERATED_BINARIES	:= build/$(PROFILE)/$(BINARY).elf \
					   build/$(PROFILE)/$(BINARY).bin \
					   build/$(PROFILE)/$(BINARY).list \
					   build/$(PROFILE)/$(BINARY).map

images: $(BINARY).images
flash: $(BINARY).flash

%.images: build/$(PROFILE)/%.bin build/$(PROFILE)/%.list build/$(PROFILE)/%.map
	@echo "*** $* images generated ***"

build/$(PROFILE)/%.list: build/$(PROFILE)/%.elf
	@echo "  OBJDUMP $*.list"
	$(Q)$(OBJDUMP) -S $< > $@

build/$(PROFILE)/%.elf: $(OBJS) $(LDSCRIPT)
	@echo "  LD      $*.elf"
	$(Q)$(LD) $(TGT_LDFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

build/$(PROFILE)/%.bin: build/$(PROFILE)/%.elf
	@echo "  OBJCOPY $*.bin"
	$(Q)$(OBJCOPY) -Obinary $< $@
	$(Q)../tools/header.py -p 0x4000 $@

$(LDSCRIPT): ${BINARY}.ld
	@echo "  CPP     ${BINARY}.ld"
	$(Q)mkdir -p $(@D)
	$(Q)$(CPP) $(TGT_CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) $< -o $@
	$(Q)sed -i '/^#/d' $@

build/$(PROFILE)/%.o: %.S
	@echo "  AS      $*.S"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

build/$(PROFILE)/%.o: %.c
	@echo "  CC      $*.c"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) $(TGT_CFLAGS) $(CFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

build/$(PROFILE)/%.o: %.cpp
	@echo "  CXX     $*.cpp"
	$(Q)mkdir -p $(@D)
	$(Q)$(CXX) $(TGT_CXXFLAGS) $(CXXFLAGS) $(TGT_CPPFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	@echo "  CLEAN"
	$(Q)$(RM) $(GENERATED_BINARIES) $(OBJS) $(OBJS:%.o=%.d)
	$(Q)$(RM) -r build

.PHONY: images clean flash elf bin list

-include $(OBJS:.o=.d)
