#
#  Created by Jeremy on 2015-08-08.
#  Copyright 2015. All rights reserved.  
#

# **************************************
# Source Path and Code
# **************************************

MACH_SSRC += \
    kode/cpu/x86_mch/cpu_head_s.S

MACH_CSRC += \
    kode/cpu/x86_mch/cpu_boot.c
    
          
# **************************************
# Build Path
# **************************************
          
BUILD_DIR += \
    $(BUILD_ROOT)/kode \
    $(BUILD_ROOT)/kode/cpu \
    $(BUILD_ROOT)/kode/cpu/x86_mch

INC_DIR += \
    -I$(SRC_ROOT)/kode/cpu/x86_mch


