# SPDX-License-Identifier: GPL-2.0-or-later

obj-y += gpio_isolation.o

# RTOS
ccflags-y += -I drivers/rtos-hal/hal/source/amp_rsc/core/include

ifeq ($(BSP_TOP), )
BSP_TOP := bsp/
endif
# Linux and U-Boot
ccflags-y += -I $(srctree)/$(BSP_TOP)drivers/amp_rsc/core/include
