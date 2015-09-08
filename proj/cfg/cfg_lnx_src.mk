#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Env.
# **************************************

KODE_CPU_LNX_DIR := x86_lnx

KODE_CPU_LNX_SRC_S := \
    cpu_head_s.gas \
    cpu_gate_s.gas

KODE_CPU_LNX_SRC_C := \
    cpu_head_c.c \
    cpu_gate_c.c \
    cpu_hd.c \
    cpu_ps2.c \
    cpu_disp_text.c \
    cpu_disp_8bit.c \
    cpu_page.c \
    cpu_task.c \
    cpu_core.c \
    cpu_time.c 

KODE_CPU_LNX_OBJ := \
    $(patsubst %.gas, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_CPU_DIR)/$(KODE_CPU_LNX_DIR)/%.o, $(KODE_CPU_LNX_SRC_S)) \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_CPU_DIR)/$(KODE_CPU_LNX_DIR)/%.o, $(KODE_CPU_LNX_SRC_C)) 


KODE_CPU_LNX_DBG := $(patsubst %.o, %.s, $(KODE_CPU_LNX_OBJ))
KODE_CPU_LNX_DBG := $(subst build, debug, $(KODE_CPU_LNX_DBG))

BUILD_DIR += $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_CPU_DIR)/$(KODE_CPU_LNX_DIR)
INC_DIR   += -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_CPU_DIR)/$(KODE_CPU_LNX_DIR)

