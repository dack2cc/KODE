#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Env.
# **************************************

KODE_NAME := kode
KODE_BIN  := $(KODE_NAME).bin
KODE_MAP  := $(KODE_NAME).map
KODE_TMP  := $(KODE_NAME).tmp
KODE_ROOT := $(KODE_NAME)

KODE_STARTUP_DIR := startup
KODE_STARTUP_SRC_S := s_head.gas
KODE_STARTUP_SRC_C := s_main.c
KODE_STARTUP_OBJ := $(patsubst %.gas, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_STARTUP_DIR)/%.o, $(KODE_STARTUP_SRC_S)) \
                    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_STARTUP_DIR)/%.o, $(KODE_STARTUP_SRC_C))

KODE_KERNEL_DIR := kernel

KODE_KERNEL_CPU_DIR := cpu
KODE_KERNEL_CPU_SRC_S := cpu_gate_s.gas
KODE_KERNEL_CPU_SRC_C := cpu_gate_c.c \
                         cpu_key.c \
                         cpu_disp.c \
                         cpu_page.c \
                         cpu_task.c \
                         cpu_core.c
KODE_KERNEL_CPU_OBJ := $(patsubst %.gas, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_CPU_DIR)/%.o, $(KODE_KERNEL_CPU_SRC_S)) \
                       $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_CPU_DIR)/%.o, $(KODE_KERNEL_CPU_SRC_C))


KODE_KERNEL_OS_DIR := os
KODE_KERNEL_OS_SRC_S := 
KODE_KERNEL_OS_SRC_C := os_cpu.c \
                        os_cfg_app.c \
                        os_core.c \
                        os_dbg.c \
                        os_flag.c \
                        os_int.c \
                        os_mem.c \
                        os_msg.c \
                        os_mutex.c \
                        os_pend_multi.c \
                        os_prio.c \
                        os_q.c \
                        os_sem.c \
                        os_stat.c \
                        os_task.c \
                        os_tick.c \
                        os_tmr.c \
                        os_time.c \
                        os_var.c 
KODE_KERNEL_OS_OBJ := $(patsubst %.gas, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_OS_DIR)/%.o, $(KODE_KERNEL_OS_SRC_S)) \
                      $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_OS_DIR)/%.o, $(KODE_KERNEL_OS_SRC_C))


KODE_KERNEL_DRV_DIR := drv
KODE_KERNEL_DRV_SRC_S := 
KODE_KERNEL_DRV_SRC_C := drv_disp.c
KODE_KERNEL_DRV_OBJ := $(patsubst %.gas, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_DRV_DIR)/%.o, $(KODE_KERNEL_DRV_SRC_S)) \
                       $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_DRV_DIR)/%.o, $(KODE_KERNEL_DRV_SRC_C))


KODE_KERNEL_LIB_DIR := lib
KODE_KERNEL_LIB_SRC_S := 
KODE_KERNEL_LIB_SRC_C := lib_pool.c
KODE_KERNEL_LIB_OBJ := $(patsubst %.gas, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR)/%.o, $(KODE_KERNEL_LIB_SRC_S)) \
                       $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR)/%.o, $(KODE_KERNEL_LIB_SRC_C))


KODE_KERNEL_KD_DIR := kd
KODE_KERNEL_KD_SRC_S := 
KODE_KERNEL_KD_SRC_C := kd_core.c \
                        kd_thread.c 
KODE_KERNEL_KD_OBJ := $(patsubst %.gas, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_KD_DIR)/%.o, $(KODE_KERNEL_KD_SRC_S)) \
                      $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_KD_DIR)/%.o, $(KODE_KERNEL_KD_SRC_C))



KODE_OBJ := $(KODE_STARTUP_OBJ) \
            $(KODE_KERNEL_CPU_OBJ) \
            $(KODE_KERNEL_OS_OBJ) \
            $(KODE_KERNEL_LIB_OBJ) \
            $(KODE_KERNEL_KD_OBJ) \
            $(KODE_KERNEL_DRV_OBJ)
            

KODE_DBG := $(patsubst %.o, %.s, $(KODE_OBJ))
KODE_DBG := $(subst build,debug, $(KODE_DBG))

BUILD_DIR += $(BUILD_ROOT)/$(KODE_ROOT) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_STARTUP_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_CPU_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_OS_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_KD_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_DRV_DIR)
                          
INC_DIR += -I$(SRC_ROOT)/$(KODE_ROOT) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/include \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_CPU_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_OS_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_KD_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_DRV_DIR)

# **************************************
# Make Rule
# **************************************

kode : _PREPARE _IMAGE_KODE

_IMAGE_KODE : $(TOOL) $(BOOT_BIN) $(KODE_BIN)
	@echo "[Build][$(TARGET_BIN)]"
	@$(BUILD_ROOT)/$(TOOL) $(DEV_FLOPPY) $(BOOT_BIN) $(BUILD_ROOT)/$(KODE_BIN) > $(TARGET_ROOT)/$(TARGET_BIN)
	@echo "[Build][$(TARGET_HD)]"
	@$(BUILD_ROOT)/$(TOOL) $(DEV_HDISK) $(BOOT_BIN) $(BUILD_ROOT)/$(KODE_BIN) > $(TARGET_ROOT)/$(TARGET_HD)

$(KODE_BIN) : $(KODE_OBJ) $(KODE_DBG)
	@echo "[Build][$(BUILD_ROOT)/$@]"
	@$(LD) $(LDFLAGS) $(KODE_OBJ) -o $(BUILD_ROOT)/$(KODE_TMP) > $(DEBUG_ROOT)/$(KODE_MAP)
	@$(OBJCP) $(OBJCPFLAGS) $(BUILD_ROOT)/$(KODE_TMP) $(BUILD_ROOT)/$(KODE_BIN)

