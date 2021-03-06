#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Env.
# **************************************

BOOT_BSD_DIR := $(SRC_ROOT)/kode/boot/x86_bsd
BOOT_BSD_SRC := boot0.S boot1.S

BOOT_BSD_BIN := $(patsubst %.S, $(BUILD_ROOT)/%.bin, $(BOOT_BSD_SRC))
BOOT_BSD_MAP := $(patsubst %.S, $(DEBUG_ROOT)/%.map, $(BOOT_BSD_SRC))

BOOT_BSD_0   := $(BUILD_ROOT)/boot0.bin

#FLAGSPLUS := -no-integrated-as

# **************************************
# Make Rule
# **************************************

boot : _PREPARE $(MKIMG) $(BOOT_BSD_BIN) $(MKSRC)
#	@echo "[Build][$(TARGET_BIN)]"
#	$(BUILD_ROOT)/$(MKIMG) $(DEV_FLOPPY)  $(BOOT_BSD_0)  $(BUILD_ROOT)/setup.bin > $(TARGET_ROOT)/$(TARGET_BIN)
#	@echo "[Build][$(TARGET_HD)]"
#	$(BUILD_ROOT)/$(MKIMG) $(DEV_HDISK)  $(BOOT_BSD_0)  $(BUILD_ROOT)/setup.bin > $(TARGET_ROOT)/$(TARGET_HD)

$(BUILD_ROOT)/%.bin : $(BOOT_BSD_DIR)/%.S
	@echo "[Compile ][$<]"
	$(CLANG) $(CLANGFLAGS) $(FLAGSPLUS) -m32 -c $< -o $@.o
	$(CLANG) $(CLANGFLAGS) -e start -Ttext 0x600 -Wl,-N,-S,-M -nostdlib -o $@.tmp $@.o > $(BOOT_BSD_MAP)
	$(OBJCP) -j .text -S -O binary $@.tmp $@
	rm $@.*

