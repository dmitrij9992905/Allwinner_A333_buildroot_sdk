# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2009 Wind River Systems, Inc.
# Tom Rix <Tom.Rix at windriver.com>

# include
ccflags-y += -I $(srctree)/drivers/sunxi_power/include

# version
VERSION_FILE := $(srctree)/drivers/sunxi_power/version
VERSION_PREFIX := SUNXI_POWER
VERSION := $(shell grep -m 1 'version:' $(VERSION_FILE) | awk '{print $$2}')
ccflags-y += -D$(VERSION_PREFIX)="\"$(VERSION)\""
$(info sunxi power version is $(VERSION))

# core
obj-$(CONFIG_SUNXI_POWER)		+= core/axp.o
obj-$(CONFIG_SUNXI_POWER)		+= core/power_manage.o
obj-$(CONFIG_SUNXI_PMU)			+= core/pmu.o
obj-$(CONFIG_SUNXI_BMU)			+= core/bmu.o
obj-$(CONFIG_SUNXI_PMU_EXT)		+= core/pmu_ext.o
obj-$(CONFIG_SUNXI_PMU_GENERAL)		+= core/pmu_general.o
obj-$(CONFIG_SUNXI_BMU_EXT)		+= core/bmu_ext.o

# pmu
ifdef CONFIG_SUNXI_PMU
obj-$(CONFIG_AXPNULL_POWER)		+= pmu/pmu_axpnull.o

obj-$(CONFIG_SUNXI_AXP152_POWER)	+= pmu/pmu_axp152.o
obj-$(CONFIG_AXP221_POWER_SUNXI)	+= pmu/pmu_axp221.o
obj-$(CONFIG_AXP81X_POWER)			+= pmu/pmu_axp81X.o
obj-$(CONFIG_AXP2101_POWER)		+= pmu/pmu_axp2101.o
obj-$(CONFIG_AXP2202_POWER)		+= pmu/pmu_axp2202.o

obj-$(CONFIG_AXP858_POWER)		+= pmu/pmu_axp858.o
obj-$(CONFIG_AXP806_POWER)		+= pmu/pmu_axp806.o
obj-$(CONFIG_AXP1530_POWER)		+= pmu/pmu_axp1530.o
obj-$(CONFIG_AXP8191_POWER)		+= pmu/pmu_axp8191.o
endif

# bmu
ifdef CONFIG_SUNXI_BMU

obj-$(CONFIG_AXP221_POWER_SUNXI)	+= bmu/bmu_axp221.o
obj-$(CONFIG_SUNXI_AXP152_POWER)	+= bmu/bmu_axp152.o
obj-$(CONFIG_AXP81X_POWER)		+= bmu/bmu_axp81X.o
obj-$(CONFIG_AXP2101_POWER)		+= bmu/bmu_axp2101.o
obj-$(CONFIG_AXP2202_POWER)		+= bmu/bmu_axp2202.o

obj-$(CONFIG_AXP515_POWER)		+= bmu/bmu_axp515.o
obj-$(CONFIG_AXP517_POWER)		+= bmu/bmu_axp517.o
obj-$(CONFIG_AXP2601_POWER)		+= bmu/bmu_axp2601.o
endif

# pmu_ext
ifdef CONFIG_SUNXI_PMU_EXT
obj-$(CONFIG_TCS4838_POWER)		+= pmu/pmu_tcs4838.o
obj-$(CONFIG_SY8827G_POWER)		+= pmu/pmu_sy8827g.o
obj-$(CONFIG_AXP1530_POWER)		+= pmu/pmu_axp1530_ext.o
endif

# pmu_general
ifdef CONFIG_SUNXI_PMU_GENERAL
obj-$(CONFIG_OCP2131_POWER)		+= pmu/pmu_ocp2131.o
obj-$(CONFIG_AW37501_POWER)		+= pmu/pmu_aw37501.o
endif

# bmu_ext
ifdef CONFIG_SUNXI_BMU_EXT
obj-$(CONFIG_ETA6973_POWER)		+= bmu/bmu_eta6973.o
obj-$(CONFIG_AXP519_POWER)		+= bmu/bmu_axp519.o
endif
