#
# Copyright (C) 2015 Jeff Kent <jeff@jkent.net>
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

CC      := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE    := $(CROSS_COMPILE)size

TARGET  := nanoboot.bin

BL1_AFILES := $(wildcard src/bl1/*.S)
BL1_CFILES := $(wildcard src/bl1/*.c)
BL1_OFILES := $(BL1_AFILES:src/bl1/%.S=build/bl1/%.o) $(BL1_CFILES:src/bl1/%.c=build/bl1/%.o)

CFILES := $(wildcard src/*.c) $(wildcard src/nanolib/*.c) src/fatfs/ff.c src/fatfs/diskio.c
OFILES := $(BL1_OFILES) $(CFILES:src/%.c=build/%.o)

ifeq ($(DEBUG),1)
	CFLAGS := -O0
else
	CFLAGS := -Os
endif

CFLAGS += -g3 -Wall -Wno-unused-function -std=gnu99 -ffreestanding -fdata-sections -ffunction-sections -nostdinc -D__KERNEL__
CFLAGS += -mlittle-endian -msoft-float -mtune=arm926ej-s
ASFLAGS := -Wa,--defsym,_entry=0 -D__ASSEMBLY__
LDFLAGS := -nostartfiles -nodefaultlibs -nostdlib -static -Wl,--gc-sections
INCLUDE := -I./src/nanolib/include -I./include -I./src

ifeq ($(V),1)
	D := @true
	Q :=
else
	D := @echo
	Q := @
endif

.SECONDARY:
.PHONY: all
all: build/$(TARGET)

-include $(shell find build -name \*.d -print)

build/%.o: src/%.S
	$(D) "AS      $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -c $(CFLAGS) $(ASFLAGS) $(INCLUDE) -MMD -MP -MF build/$*.d $< -o $@

build/%.o: src/%.c
	$(D) "CC      $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -c $(CFLAGS) $(INCLUDE) -MMD -MP -MF build/$*.d $< -o $@

build/%.ld: src/%.ld.S
	$(D) "CPP     $<"
	$(Q)mkdir -p $(@D)
	$(Q)$(CC) -P -E $(INCLUDE) -MMD -MP -MF $(@:=.d) $< -o $@

build/%.elf: build/$(TARGET:.bin=.ld) $(OFILES)
	$(D) "LD      $@"
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-M,-Map,build/$(TARGET:.bin=.map) -T $^ -lgcc -o $@

build/%.bin: build/%.elf
	$(D) "OBJDUMP $<"
	$(Q)$(OBJDUMP) -S -j .text $< > $(@:.bin=.dis) || rm $(@:.bin=.dis)
	$(D) "OBJCOPY $<"
	$(Q)$(OBJCOPY) -S -I elf32-littlearm -O binary $< $@
	$(Q)$(SIZE) $<

.PHONY: clean
clean:
	$(Q)rm -rf build
