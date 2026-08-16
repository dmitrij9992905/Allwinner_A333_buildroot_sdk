# SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_SMP), y)
	obj-y += helper_linux.o
else
ifeq ($(CONFIG_KERNEL_FREERTOS), y)
	obj-y += helper_rtos.o
else
	obj-y += fdt_helper.o
	obj-y += helper_uboot.o
endif
endif

obj-y += shm_helper.o
obj-y += mgr_helper.o
obj-y += gpio_helper.o
