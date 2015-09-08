#
#  Created by Jeremy on 2015-08-08.
#  Copyright 2015. All rights reserved.  
#


# **************************************
# Object
# **************************************t

MACH_TARGET   := kodext
MACH_TARGET_A := $(MACH_TARGET).a
MACH_TARGET_O := $(MACH_TARGET).o
MACH_TARGET_M := $(MACH_TARGET).map

MACH_OBJ := $(patsubst %.S, $(BUILD_ROOT)/%.o, $(MACH_SSRC)) \
            $(patsubst %.c, $(BUILD_ROOT)/%.o, $(MACH_CSRC))

MACH_DBG := $(patsubst %.S, $(DEBUG_ROOT)/%.s, $(MACH_SSRC)) \
            $(patsubst %.c, $(DEBUG_ROOT)/%.s, $(MACH_CSRC))

MACH_OBJ += $(KODE_KERNEL_LIB_OBJ)
MACH_DBG += $(KODE_KERNEL_LIB_DBG)

# **************************************
# Rule
# **************************************t


mach : _PREPARE _PREBUILD $(MACH_OBJ) $(MACH_DBG)
	@echo "[package -> $(MACH_TARGET_A)]"
	@$(AR) cru $(BUILD_ROOT)/$(MACH_TARGET_A) $(MACH_OBJ)
	@echo "[link  ---> $(MACH_TARGET_O)]"
	@$(LD) -melf_i386 -u _start -r -o $(BUILD_ROOT)/$(MACH_TARGET_O) $(BUILD_ROOT)/$(MACH_TARGET_A)
	@echo "[build ---> $(MACH_TARGET)]"
	@$(LD) -melf_i386 -M --defsym _START=0xC0100000 --defsym _START_MAP=0x100000 -T '..'/cfg/cfg_mch_ldscript -o $(TARGET_ROOT)/$(MACH_TARGET) $(BUILD_ROOT)/$(MACH_TARGET_O) > $(DEBUG_ROOT)/$(MACH_TARGET_M)

_PREBUILD :
	@echo "[prepare ]"
	@mawk -f $(SRC_ROOT)/tools/gensym.awk $(SRC_ROOT)/kode/cpu/x86_mch/cpu_asm.sym > $(BUILD_ROOT)/cpu_asm.symc
	@$(CC) -I../../src/kode/cpu/x86_mch -I../../src/kode/cpu -m32 -nostdinc -Wall -fgnu89-inline -fno-stack-protector -mno-3dnow -mno-mmx -mno-sse -mno-sse2 -g -O2 -S -x c -o $(BUILD_ROOT)/cpu_asm.symc.o $(BUILD_ROOT)/cpu_asm.symc
	@sed < $(BUILD_ROOT)/cpu_asm.symc.o > $(DEBUG_ROOT)/cpu_asm_symc.h \
	-e 's/^[^*].*$$//'			\
	-e 's/^[*]/#define/'			\
	-e 's/mAgIc[^-0-9]*//'

machdbg :
#	@echo $(MACH_OBJ)
#	@echo $(MACH_DBG)


