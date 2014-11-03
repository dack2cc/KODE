#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Enviornment
# **************************************

-include ../cfg/cfg_env.mk

# **************************************
# Sub Target
# **************************************

-include ../cfg/cfg_boot.mk
-include ../cfg/cfg_kode.mk

DEBUG_DIR  += $(subst build,debug,$(BUILD_DIR))


# **************************************
# Make Rule
# **************************************

all : boot

# **************************************
# BSD Make Rule
# **************************************

CC := clang

CFLAGS := -O2 -pipe -DVOLUME_SERIAL -DPXE -DFLAGS=0x8f -DTICKS=0xb6 -DCOMSPEED="7 << 5 + 3" -march=i386 -ffreestanding  \
          -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mno-sse3 -msoft-float -m32 -std=gnu99 -Qunused-arguments

BOOT_DIR := ../../src/kode/boot/x86_bsd
BOOT_SRC := boot0.S

bsd:
	$(CC) $(CFLAGS) -no-integrated-as -m32 -c $(BOOT_DIR)/$(BOOT_SRC)
	#$(CC) $(CFLAGS) -e start -Ttext 0x00 -Wl,-N,-S -nostdlib -o boot0 boot0.o
	#$(LD) $(LDFLAGS) boot0.o -o boot0
	$(LD) -static -N --gc-sections -e start -Ttext 0x0000 -o boot0.out boot0.o
	$(OBJCP) -S -O binary ./boot0.out ./boot0.bin

