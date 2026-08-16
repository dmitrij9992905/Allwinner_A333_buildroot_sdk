# SPDX-License-Identifier: GPL-2.0-only
ifeq ($(CONFIG_AW_HW_CPUFREQ),m)
obj-$(CONFIG_CPU_FREQ) += cpufreq-linux-$(VERSION).$(PATCHLEVEL)/
else
obj-$(CONFIG_AW_CPUFREQ_DT) += cpufreq-linux-$(VERSION).$(PATCHLEVEL)/
endif
obj-$(CONFIG_ARM_AW_SUN50I_CPUFREQ_NVMEM) += sun50i-cpufreq-nvmem.o
obj-$(CONFIG_AW_CPUFREQ_TEST) += vf-test.o
ccflags-y := -DDYNAMIC_DEBUG_MODULE
