#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Env.
# **************************************

KODE_ROOT := kode
KODE_CPU_DIR := cpu

KODE_STARTUP_DIR := startup
KODE_STARTUP_SRC_C := s_main.c
KODE_STARTUP_OBJ := \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_STARTUP_DIR)/%.o, $(KODE_STARTUP_SRC_C))


KODE_KERNEL_DIR := kernel

KODE_KERNEL_OS_DIR   := os
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
KODE_KERNEL_OS_OBJ := \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_OS_DIR)/%.o, $(KODE_KERNEL_OS_SRC_C))


KODE_KERNEL_DRV_DIR   := drv
KODE_KERNEL_DRV_SRC_C := drv_disp.c \
                         drv_lock.c \
                         drv_mice.c \
                         drv_key.c \
                         drv_blk.c \
                         drv_gfx.c \
                         drv_hd.c \
                         drv_rd.c 
KODE_KERNEL_DRV_OBJ := \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_DRV_DIR)/%.o, $(KODE_KERNEL_DRV_SRC_C))


KODE_KERNEL_LIB_DIR   := lib
KODE_KERNEL_LIB_SRC_C := lib_pool.c \
                         lib_mem.c \
                         lib_ascii.c \
                         lib_math.c \
                         lib_str.c
KODE_KERNEL_LIB_OBJ := \
    $(patsubst %.c, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR)/%.o, $(KODE_KERNEL_LIB_SRC_C))
KODE_KERNEL_LIB_DBG := \
    $(patsubst %.c, $(DEBUG_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR)/%.s, $(KODE_KERNEL_LIB_SRC_C))

KODE_KERNEL_FS_DIR   := fs
KODE_KERNEL_FS_SRC_C := fs_super.c \
                        fs_inode.c \
                        fs_table.c 
KODE_KERNEL_FS_OBJ := \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_FS_DIR)/%.o, $(KODE_KERNEL_FS_SRC_C))

KODE_KERNEL_KD_DIR   := kd
KODE_KERNEL_KD_SRC_C := kd_core.c \
                        kd_thread.c \
                        kd_event.c \
                        kd_time.c \
                        kd_file.c \
                        kd_proc.c
KODE_KERNEL_KD_OBJ := \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_KD_DIR)/%.o, $(KODE_KERNEL_KD_SRC_C))

KODE_KERNEL_FONT_DIR := font
KODE_KERNEL_FONT_SRC := font_hankaku.c
KODE_KERNEL_FONT_OBJ := \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_FONT_DIR)/%.o, $(KODE_KERNEL_FONT_SRC))

KODE_KERNEL_GUI_DIR := gui
KODE_KERNEL_GUI_SRC := gui_core.c \
                       gui_mice.c \
                       gui_win.c \
                       gui_log.c \
                       gui_bg.c
KODE_KERNEL_GUI_OBJ := \
    $(patsubst %.c,   $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_GUI_DIR)/%.o, $(KODE_KERNEL_GUI_SRC))


KODE_SYSTEM_DIR := system
KODE_SYSTEM_LIB_DIR := lib

KODE_SYSTEM_LIB_KD_DIR := kd
KODE_SYSTEM_LIB_KD_SRC := kd_syscall.c \
                          kd_lib.c
KODE_SYSTEM_LIB_KD_OBJ := \
    $(patsubst %.c, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_SYSTEM_DIR)/$(KODE_SYSTEM_LIB_DIR)/$(KODE_SYSTEM_LIB_KD_DIR)/%.o, $(KODE_SYSTEM_LIB_KD_SRC))

KODE_SYSTEM_LIB_STD_DIR := std
KODE_SYSTEM_LIB_STD_SRC := vsprintf.c \
                           printf.c 
KODE_SYSTEM_LIB_STD_OBJ := \
    $(patsubst %.c, $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_SYSTEM_DIR)/$(KODE_SYSTEM_LIB_DIR)/$(KODE_SYSTEM_LIB_STD_DIR)/%.o, $(KODE_SYSTEM_LIB_STD_SRC))


KODE_OBJ :=  \
            $(KODE_CPU_OBJ) \
            $(KODE_KERNEL_OS_OBJ) \
            $(KODE_KERNEL_DRV_OBJ) \
            $(KODE_KERNEL_LIB_OBJ) \
            $(KODE_KERNEL_FS_OBJ) \
            $(KODE_KERNEL_KD_OBJ) \
            $(KODE_KERNEL_FONT_OBJ) \
            $(KODE_KERNEL_GUI_OBJ) \
            $(KODE_SYSTEM_LIB_KD_OBJ) \
            $(KODE_SYSTEM_LIB_STD_OBJ) \
            $(KODE_STARTUP_OBJ)


KODE_DBG := $(patsubst %.o, %.s, $(KODE_OBJ))
KODE_DBG := $(subst build,debug, $(KODE_DBG))


BUILD_DIR += $(BUILD_ROOT)/$(KODE_ROOT) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_CPU_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_CPU_DIR)/$(KODE_CPU_LNX_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_STARTUP_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_CPU_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_OS_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_KD_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_DRV_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_FS_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_FONT_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_GUI_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_SYSTEM_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_SYSTEM_DIR)/$(KODE_SYSTEM_LIB_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_SYSTEM_DIR)/$(KODE_SYSTEM_LIB_DIR)/$(KODE_SYSTEM_LIB_KD_DIR) \
             $(BUILD_ROOT)/$(KODE_ROOT)/$(KODE_SYSTEM_DIR)/$(KODE_SYSTEM_LIB_DIR)/$(KODE_SYSTEM_LIB_STD_DIR)


INC_DIR += -I$(SRC_ROOT)/$(KODE_ROOT) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/include \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_CPU_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_OS_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_LIB_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_KD_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_DRV_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_FS_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_FONT_DIR) \
           -I$(SRC_ROOT)/$(KODE_ROOT)/$(KODE_KERNEL_DIR)/$(KODE_KERNEL_GUI_DIR)





