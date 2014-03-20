#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Env.
# **************************************

BOOT_DIR := ../src/kode/boot
BOOT_SRC := boot.nas \
            setup.nas

BOOT_BIN := $(patsubst %.nas, $(BUILD_ROOT)/%.bin, $(BOOT_SRC))

