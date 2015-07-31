#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Target
# **************************************

TOOLS_DIR   := tools
MKIMG       := mkimg
MKFS        := mkfs
MKSRC       := mksrc

INSTALL_DIR := ../../bochs

TARGET_ROOT := ./bin
TARGET_DIR  := $(TARGET_ROOT)
TARGET_BIN  := boot.img
TARGET_HD   := disk.img
TARGET_FS   := ramfs.img

BUILD_ROOT := ./build
BUILD_DIR  := $(BUILD_ROOT)

DEBUG_ROOT := ./debug
DEBUG_DIR  :=

DEV_FLOPPY := FLOPPY
DEV_HDISK  := HDISK

SRC_ROOT := ../../src
INC_DIR  := 

BOOT_DIR := 

# **************************************
# Networking Assembly Build
# **************************************

NASM := nasm
NASMFLAGS := -f bin -w+orphan-labels

# **************************************
# GNU Tools Build
# **************************************

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
CLANG := clang

ASFLAGS    := -march=i386+387 --32
CFLAGS     := -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin
CXXFLAGS   := $(CFLAGS)
LDFLAGS    := -s -x -M -Ttext 0x00000
CLANGFLAGS := -O2 -pipe -DVOLUME_SERIAL -DPXE -DFLAGS=0x8f -DTICKS=0xb6 -DCOMSPEED="7 << 5 + 3" -march=i386 -ffreestanding  \
              -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mno-sse3 -msoft-float -m32 -std=gnu99
OBJCPFLAGS := -O binary

# **************************************
# Make Rule
# **************************************

_PREPARE :
	@for DIR in $(BUILD_DIR) $(DEBUG_DIR) $(TARGET_DIR) ; do \
	if [ ! -d $$DIR ] ; then mkdir $$DIR ; fi \
	done

_IMAGE : $(MKIMG) $(BOOT_BIN) $(MKSRC)
	@echo "[Build][$(TARGET_BIN)]"
	@$(BUILD_ROOT)/$(MKIMG) $(DEV_FLOPPY) $(BOOT_BIN) > $(TARGET_ROOT)/$(TARGET_BIN)
	@echo "[Build][$(TARGET_HD)]"
	@$(BUILD_ROOT)/$(MKIMG) $(DEV_HDISK) $(BOOT_BIN) > $(TARGET_ROOT)/$(TARGET_HD)

$(MKIMG) :
	@echo "[Compile ][$(SRC_ROOT)/$(TOOLS_DIR)/$(MKIMG).c]"
	@$(CC) $(SRC_ROOT)/$(TOOLS_DIR)/$(MKIMG).c -o $(BUILD_ROOT)/$(MKIMG)
	@chmod o+x $(BUILD_ROOT)/$(MKIMG)

$(MKFS) :
	@echo "[Compile ][$(SRC_ROOT)/$(TOOLS_DIR)/$(MKFS).c]"
	@$(CC) $(SRC_ROOT)/$(TOOLS_DIR)/$(MKFS).c -o $(BUILD_ROOT)/$(MKFS)
	@chmod o+x $(BUILD_ROOT)/$(MKFS)
	
$(MKSRC) :
	@echo "[Compile ][$(SRC_ROOT)/$(TOOLS_DIR)/$(MKSRC).c]"
	@$(CC) $(SRC_ROOT)/$(TOOLS_DIR)/$(MKSRC).c -o $(BUILD_ROOT)/$(MKSRC)
	@chmod o+x $(BUILD_ROOT)/$(MKSRC)

$(BUILD_ROOT)/%.bin : $(BOOT_DIR)/%.nas
	@echo "[Assemble][$<]"
	@$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.gas
	@echo "[Assemble][$<]"
	@$(CC) $(CFLAGS) -x assembler -c $< -o $@
#	@$(AS) $(ASFLAGS) $< -o $@

#.S.o:	
$(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.S
	@echo "[Assemble][$<]"
	@$(CC) $(CFLAGS) -x assembler -c $< -o $@
#	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.c
	@echo "[Compile ][$<]"
	@$(CC) $(CFLAGS) $(INC_DIR) -m32 -nostdinc -c $< -o $@

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.gas
	@echo "[Generate][$@]"
	@cp -fr $< $@

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.S
	@echo "[Generate][$@]"
	@cp -fr $< $@

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.c
#	@echo "[Generate][$@]"
	@$(CC) $(CFLAGS) $(INC_DIR) -m32 -nostdinc -w -S $< -o $@

clean :
	@echo "[clean...]"
	@rm -fr $(BUILD_ROOT)
	@rm -fr $(DEBUG_ROOT)
	@rm -fr $(TARGET_ROOT)
	@rm -fr $(INSTALL_DIR)/$(TARGET_DIR)
	
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

install :
	@cp -fr $(TARGET_DIR) $(INSTALL_DIR)
	
run :
	@cp -fr $(TARGET_DIR) $(INSTALL_DIR)
	@cd $(INSTALL_DIR)
	@bochs -q -f boot_win32.bxrc
	@cd -

