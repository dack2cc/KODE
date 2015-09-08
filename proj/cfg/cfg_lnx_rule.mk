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

TARGET_BIN  := boot.img
TARGET_HD   := disk.img
TARGET_FS   := ramfs.img

DEV_FLOPPY := FLOPPY
DEV_HDISK  := HDISK

KODE_NAME := kode
KODE_BIN  := $(KODE_NAME).bin
KODE_MAP  := $(KODE_NAME).map
KODE_TMP  := $(KODE_NAME).tmp


# **************************************
# Build Env.
# **************************************

KODE_OBJ += $(KODE_CPU_LNX_OBJ)
KODE_DBG += $(KODE_CPU_LNX_DBG)


# **************************************
# Make Rule
# **************************************

kode : _PREPARE _IMAGE_KODE

_IMAGE_KODE : $(MKIMG) $(MKFS) $(BOOT_BIN) $(KODE_BIN)
	@echo "[Build][$(TARGET_FS)]"
	@$(BUILD_ROOT)/$(MKFS) > $(TARGET_ROOT)/$(TARGET_FS)
	@echo "[Build][$(TARGET_BIN)]"
	@$(BUILD_ROOT)/$(MKIMG) $(DEV_FLOPPY) $(BOOT_BIN) $(BUILD_ROOT)/$(KODE_BIN) $(TARGET_ROOT)/$(TARGET_FS) > $(TARGET_ROOT)/$(TARGET_BIN)
	@echo "[Build][$(TARGET_HD)]"
	@$(BUILD_ROOT)/$(MKIMG) $(DEV_HDISK) $(BOOT_BIN) $(BUILD_ROOT)/$(KODE_BIN) $(TARGET_ROOT)/$(TARGET_FS) > $(TARGET_ROOT)/$(TARGET_HD)

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

$(KODE_BIN) : $(KODE_OBJ) $(KODE_DBG)
	@echo "[Build][$(BUILD_ROOT)/$@]"
	@$(LD) $(LDFLAGS) $(KODE_OBJ) -o $(BUILD_ROOT)/$(KODE_TMP) > $(DEBUG_ROOT)/$(KODE_MAP)
	@$(OBJCP) $(OBJCPFLAGS) $(BUILD_ROOT)/$(KODE_TMP) $(BUILD_ROOT)/$(KODE_BIN)



