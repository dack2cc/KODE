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

MACH_OBJS := $(patsubst %.S, $(BUILD_ROOT)/%.o, $(MACH_SSRC)) \
             $(patsubst %.c, $(BUILD_ROOT)/%.o, $(MACH_CSRC))

# **************************************
# Rule
# **************************************t


mach : _PREPARE $(MACH_OBJS)
	$(AR) cru $(BUILD_ROOT)/$(MACH_TARGET_A) $(MACH_OBJS)
	$(LD) -melf_i386 -u _start -r -o $(BUILD_ROOT)/$(MACH_TARGET_O) $(BUILD_ROOT)/$(MACH_TARGET_A)
	$(LD) -melf_i386 --defsym _START=0xC0100000 --defsym _START_MAP=0x100000 -T '..'/cfg/cfg_mach_ldscript -o $(BUILD_ROOT)/$(MACH_TARGET) $(BUILD_ROOT)/$(MACH_TARGET_O)



