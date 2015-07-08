#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Env.
# **************************************

BOOT_DIR := $(SRC_ROOT)/kode/boot/x86_lnx
BOOT_SRC := boot.nas \
            setup.nas

BOOT_BIN := $(patsubst %.nas, $(BUILD_ROOT)/%.bin, $(BOOT_SRC))

# **************************************
# Make Rule
# **************************************

boot : _PREPARE $(MKIMG) $(BOOT_BIN) $(MKSRC)
	@echo "[Build][$(TARGET_BIN)]"
	@$(BUILD_ROOT)/$(MKIMG) $(DEV_FLOPPY) $(BOOT_BIN) > $(TARGET_ROOT)/$(TARGET_BIN)
	@echo "[Build][$(TARGET_HD)]"
	@$(BUILD_ROOT)/$(MKIMG) $(DEV_HDISK) $(BOOT_BIN) > $(TARGET_ROOT)/$(TARGET_HD)

$(BUILD_ROOT)/%.bin : $(BOOT_DIR)/%.nas
	@echo "[Assemble][$<]"
	@$(NASM) $(NASMFLAGS) $< -o $@

