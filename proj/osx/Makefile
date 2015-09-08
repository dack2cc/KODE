#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

#  NOTE:
#  We need brew for Mac OSX, which can get from the site below.
#    http://brew.sh
#
#  And then we need the gobjcopy, which can get from brew.
#    brew install crosstool-ng

# **************************************
# Build Enviornment
# **************************************

-include ../cfg/cfg_env.mk

OBJCP := gobjcopy
#GAS:=as
GAS:=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/libexec/as/i386/as

ASFLAGS    := -arch i386
CFLAGS     := -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin \
              -arch i386 -march=i486 -nostdinc -fno-stack-protector -D_OSX_
CXXFLAGS   := $(CFLAGS)
LDFLAGS    := -x -M -image_base 0x0000 -arch i386 -Z -map $(DEBUG_ROOT)/kode.map \
              -static -u _s_main -e _s_main
              #-static -u startup_32 -e startup_32

OBJCPFLAGS := -O binary 

# **************************************
# Sub Target
# **************************************

-include ../cfg/cfg_kode_src.mk
-include ../cfg/cfg_boot_lnx.mk
-include ../cfg/cfg_lnx_src.mk
-include ../cfg/cfg_lnx_rule.mk

DEBUG_DIR  += $(subst build,debug,$(BUILD_DIR))

# **************************************
# Make Rule
# **************************************

all : boot
