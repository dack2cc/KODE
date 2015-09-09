#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Enviornment
# **************************************

-include ../cfg/cfg_env.mk

ASFLAGS    := -march=i386+387 --32 --x32
CFLAGS     := -Wall -O -fstrength-reduce -fomit-frame-pointer \
              -finline-functions -fno-builtin \
              -m32 -fleading-underscore -fno-stack-protector \
              -D__ELF__=1 -D__STD__
CXXFLAGS   := $(CFLAGS)
LDFLAGS    := -s -x -M -Ttext 0x00000 -m32 
LDFLAGS    := -m elf_i386 --format=elf32-i386 
#--entry=startup_32 --format=elf32-i386
OBJCPFLAGS := -O binary
CLANG      := $(CC)

INC_DIR   += -I$(DEBUG_ROOT)

# **************************************
# Sub Target
# **************************************

-include ../cfg/cfg_kode_src.mk
-include ../cfg/cfg_boot_lnx.mk
#-include ../cfg/cfg_boot_bsd.mk
#-include ../cfg/cfg_boot_osx.mk
#-include ../cfg/cfg_lnx_src.mk
#-include ../cfg/cfg_lnx_rule.mk

-include ../cfg/cfg_mch_src.mk
-include ../cfg/cfg_mch_rule.mk

DEBUG_DIR  += $(subst build,debug,$(BUILD_DIR))




