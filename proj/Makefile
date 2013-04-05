#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Target
# **************************************

TARGET := boot.img
TOOL   := tool

BUILD_ROOT := ./build
BUILD_DIR  := $(BUILD_ROOT)

DEBUG_ROOT := ./debug
DEBUG_DIR  :=

BOOT_DEV := FLOPPY

# **************************************
# Networking Assembly Build
# **************************************

BOOT_DIR := ../src/kode/boot
BOOT_SRC := boot.nas \
            setup.nas

BOOT_BIN := $(patsubst %.nas, $(BUILD_ROOT)/%.bin, $(BOOT_SRC))

NASM := nasm
NASMFLAGS := -f bin -w+orphan-labels

# **************************************
# GNU Tools Build
# **************************************

SRC_ROOT := ../src
INC_DIR  := 

ASFLAGS    += -march=i386+387 --32
CFLAGS     += -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin
CXXFLAGS   += $(CFLAGS)
LDFLAGS    += -s -x -M -Ttext 0x00000
#LDFLAGS    += -m elf_i386
OBJCPFLAGS += -O binary

OBJCP := objcopy
GAS   := as
GLD   := ld
GAR   := ar
GCC   := gcc
GXX   := g++
AS	  := $(GAS)
AR    := $(GAR)
LD	  := $(GLD)
CC    := $(GCC)
CXX   := $(GXX)

# **************************************
# Make Rule
# **************************************

all : _PREPARE _IMAGE

_PREPARE :
	@for DIR in $(BUILD_DIR) $(DEBUG_DIR) ; do \
	if [ ! -d $$DIR ] ; then mkdir $$DIR ; fi \
	done

_IMAGE : $(TOOL) $(BOOT_BIN)
	@echo "[Build][$(TARGET)]"
	@$(BUILD_ROOT)/$(TOOL) $(BOOT_DEV) $(BOOT_BIN) > ./$(TARGET)

$(TOOL) :
	@echo "[Compile ][../src/tools/build.c]"
	@$(CC) $(CFLAGS) ../src/tools/build.c -o $(BUILD_ROOT)/$(TOOL)
	@chmod o+x $(BUILD_ROOT)/$(TOOL)

$(BUILD_ROOT)/%.bin : $(BOOT_DIR)/%.nas
	@echo "[Assemble][$<]"
	@$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.gas
	@echo "[Assemble][$<]"
	@$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.c
	@echo "[Compile ][$<]"
	@$(CC) $(CFLAGS) $(INC_DIR) -m32 -nostdinc -c $< -o $@

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.gas
#	@echo "[Generate][$@]"
	@cp -fr $< $@

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.c
#	@echo "[Generate][$@]"
	@$(CC) $(CFLAGS) $(INC_DIR) -m32 -nostdinc -w -S $< -o $@

clean :
	@echo "[clean...]"
	@rm -fr $(BUILD_ROOT)
	@rm -fr $(DEBUG_ROOT)
	@rm -fr ./$(TARGET)
	
debug :
	@echo "[BOOT_BIN][$(BOOT_BIN)]"
	@echo "[BUILD_DIR][$(BUILD_DIR)]"
	@echo "[DEBUG_DIR][$(DEBUG_DIR)]"

debug_example :
	@echo "[EXAMPLE_OBJ][$(EXAMPLE_OBJ)]"
	@echo "[EXAMPLE_DBG][$(EXAMPLE_DBG)]"

debug_linux :
	@echo "[LINUX_OBJ][$(LINUX_OBJ)]"
	@echo "[LINUX_DBG][$(LINUX_DBG)]"

debug_kode :
	@echo "[KODE_OBJ][$(KODE_OBJ)]"
	@echo "[KODE_DBG][$(KODE_DBG)]"

# **************************************
# Sub Target
# **************************************

-include cfg_example.mk
-include cfg_linux.mk
-include cfg_kode.mk

DEBUG_DIR  += $(subst build,debug,$(BUILD_DIR))

