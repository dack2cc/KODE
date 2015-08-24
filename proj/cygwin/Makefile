#
#  Created by Jeremy on 2012-11-27.
#  Copyright 2012. All rights reserved.  
#

# **************************************
# Build Enviornment
# **************************************

-include ../cfg/cfg_env.mk

CLANGFLAGS += -Qunused-arguments

# **************************************
# Sub Target
# **************************************

-include ../cfg/cfg_boot_lnx.mk
#-include ../cfg/cfg_boot_bsd.mk
-include ../cfg/cfg_lnx_src.mk
-include ../cfg/cfg_lnx_rule.mk

DEBUG_DIR  += $(subst build,debug,$(BUILD_DIR))


# **************************************
# Make Rule
# **************************************

all : boot

