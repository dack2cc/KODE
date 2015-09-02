#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Target
# **************************************

INSTALL_DIR := ../../bochs

TARGET_ROOT := ./bin
TARGET_DIR  := $(TARGET_ROOT)

BUILD_ROOT  := ./build
BUILD_DIR   := $(BUILD_ROOT)

DEBUG_ROOT  := ./debug
DEBUG_DIR   :=

SRC_ROOT    := ../../src
INC_DIR     := 

BOOT_DIR    := 

# **************************************
# Networking Assembly Build
# **************************************

NASM  := nasm
NASMFLAGS += -f bin -w+orphan-labels

# **************************************
# GNU Tools Build
# **************************************

OBJCP  := objcopy
GAS    := as
GLD    := ld
GAR    := ar
GCC    := gcc
GXX    := g++
AS     := $(GAS)
AR     := $(GAR)
LD     := $(GLD)
CC     := $(GCC)
CXX    := $(GXX)
CLANG  := clang
ASTYLE := astyle
DTOU   := dos2unix

ASFLAGS    := -march=i386+387 --32
CFLAGS     := -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -fno-builtin
CXXFLAGS   := $(CFLAGS)
LDFLAGS    := -s -x -M -Ttext 0x00000
CLANGFLAGS := -O2 -pipe -DVOLUME_SERIAL -DPXE -DFLAGS=0x8f -DTICKS=0xb6 -DCOMSPEED="7 << 5 + 3" -march=i386 -ffreestanding  \
              -mno-mmx -mno-3dnow -mno-sse -mno-sse2 -mno-sse3 -msoft-float -m32 -std=gnu99
OBJCPFLAGS  := -O binary
ASTYLE_FILE := $(shell find $(SRC_ROOT) -name *.c) \
               $(shell find $(SRC_ROOT) -name *.h)
ASTYLE_OPT  += --mode=c -A3 -z2 -s4 -f -n -p 

# **************************************
# Make Rule
# **************************************

_PREPARE :
	@for DIR in $(BUILD_DIR) $(DEBUG_DIR) $(TARGET_DIR) ; do \
	if [ ! -d $$DIR ] ; then mkdir -p $$DIR ; fi \
	done

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
	@$(CC) $(CFLAGS) $(INC_DIR) -c $< -o $@
#	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD_ROOT)/%.o : $(SRC_ROOT)/%.c
	@echo "[Compile ][$<]"
	@$(CC) $(CFLAGS) $(INC_DIR) -m32 -nostdinc -c $< -o $@
#	@set -e ; cp $(BUILD_ROOT)/$*.d $(BUILD_ROOT)/$*.dep; \
#        sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
#        -e '/^$$/ d' -e 's/$$/ :/' < $(BUILD_ROOT)/$*.d >> $(BUILD_ROOT)/$*.dep; \
#        rm -f $(BUILD_ROOT)/$*.d

$(BUILD_ROOT)/%.o : $(SRC_DIR)/%.cpp
	@echo [Compiling][$*.cpp]
	@$(GXX) $(CFLAGS) $(INT_INCLUDE) $(EXT_INCLUDE) -g -c -MD  -o $@ $<
#	@set -e ; cp $(BUILD_ROOT)/$*.d $(BUILD_ROOT)/$*.dep; \
#        sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
#        -e '/^$$/ d' -e 's/$$/ :/' < $(BUILD_ROOT)/$*.d >> $(BUILD_ROOT)/$*.dep; \
#        rm -f $(BUILD_ROOT)/$*.d

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.gas
	@echo "[Generate][$@]"
	@cp -fr $< $@

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.S
	@echo "[Generate][$@]"
	@cp -fr $< $@

$(DEBUG_ROOT)/%.s : $(SRC_ROOT)/%.c
#	@echo "[Generate][$@]"
	@$(CC) $(CFLAGS) $(INC_DIR) -m32 -nostdinc -w -S $< -o $@

format:
	@for FILE in $(ASTYLE_FILE) ; do \
	$(ASTYLE) $(ASTYLE_OPT) $$FILE ;\
	$(DTOU) $$FILE ;\
	done
	@$(DTOU) ../cfg/*.*
	@$(DTOU) ./Makefile

clean :
	@echo "[clean...]"
	@rm -fr $(BUILD_ROOT)
	@rm -fr $(DEBUG_ROOT)
	@rm -fr $(TARGET_ROOT)
	@rm -fr $(INSTALL_DIR)/$(TARGET_DIR)
	
debug :
#	@echo "[INC_DR][$(INC_DIR)]"
#	@echo "[BOOT_BIN][$(BOOT_BIN)]"
#	@echo "[BUILD_DIR][$(BUILD_DIR)]"
#	@echo "[DEBUG_DIR][$(DEBUG_DIR)]"

install :
	@cp -fr $(TARGET_DIR) $(INSTALL_DIR)
	
run :
	@cp -fr $(TARGET_DIR) $(INSTALL_DIR)
	@cd $(INSTALL_DIR)
	@bochs -q -f boot_win32.bxrc
	@cd -

