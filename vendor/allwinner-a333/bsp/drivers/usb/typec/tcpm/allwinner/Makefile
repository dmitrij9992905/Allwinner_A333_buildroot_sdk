# SPDX-License-Identifier: GPL-2.0
#
# power
ccflags-y += -I $(srctree)/bsp/drivers/power/include

obj-$(CONFIG_AW_TYPEC_TCPCI)		+= typec_tcpci.o
typec_tcpci-objs			:= tcpci.o
obj-$(CONFIG_TYPEC_AXP517)		+= tcpci_axp517.o
