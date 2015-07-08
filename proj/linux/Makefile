#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Enviornment
# **************************************

-include ../cfg/cfg_env.mk

ASFLAGS    := -march=i386+387 --32 --x32
CFLAGS     := -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin \
              -m32
CXXFLAGS   := $(CFLAGS)
LDFLAGS    := -s -x -M -Ttext 0x00000 -m32 
LDFLAGS    := -m elf_i386 --format=elf32-i386 
#--entry=startup_32 --format=elf32-i386
OBJCPFLAGS := -O binary
CLANG      := $(CC)

# **************************************
# Sub Target
# **************************************

-include ../cfg/cfg_boot_lnx.mk
#-include ../cfg/cfg_boot_bsd.mk
#-include ../cfg/cfg_boot_osx.mk
-include ../cfg/cfg_kode.mk

DEBUG_DIR  += $(subst build,debug,$(BUILD_DIR))


# **************************************
# Make Rule
# **************************************

all : boot

# **************************************
# TEST MACH
# **************************************

#CCAS = gcc
#DEFS = -DHAVE_CONFIG_H
#DEFAULT_INCLUDES = -I. -I$(srcdir)
#srcdir = ../../src
#top_srcdir = ../../src
#INCLUDES = 
#CPPFLAGS = 

#AM_CFLAGS = -fno-builtin-log -Wall -fno-strict-aliasing \
#    $(am__append_1) $(am__append_3) $(am__append_124)
#am__append_1 = -fno-stack-protector
#am__append_3 = -fno-omit-frame-pointer -fno-optimize-sibling-calls
#am__append_124 = -mno-3dnow -mno-mmx -mno-sse -mno-sse2

#CFLAGS = -g -O2
#CCASFLAGS = -g -O2

#AM_CPPFLAGS = -nostdinc -imacros config.h -I$(systype) -I. \
#    -I$(top_srcdir)/$(systype) \
#    -I$(top_srcdir)/$(systype)/include/mach/sa \
#    -I$(top_srcdir)/include

#systype = i386

#AM_CCASFLAGS = 

#CPPASCOMPILE = $(CCAS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
#    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CCASCFLAGS) $(CCASFLAGS)

#DEPDIR = deps

#am__mv = mv -f

#OBJEXT = o

#.S.o:
#	depbase=`echo $@ | sed 's|[^/]*$$|$(DEPDIR/&|;s|\.o$$||'`;\
#	$(CPPASCOMPILE) -MT $@ -MD -MP MF $$depbase.Tpo -c -o $@ $< &&\
#	$(am__mv) $$depbase.Tpo $$depbase.Po
#

