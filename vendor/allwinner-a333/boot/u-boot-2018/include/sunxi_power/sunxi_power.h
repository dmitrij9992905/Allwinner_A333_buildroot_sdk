/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */

#ifndef _SUNXI_PMIC_H
#define _SUNXI_PMIC_H

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <sys_config.h>

int sunxi_power_probe(void);
int sunxi_power_set_power_off(void);
int sunxi_power_set_restart(void);

int sunxi_update_power_info(void);
int sunxi_power_startup_status_handle(void);

int sunxi_get_power_info(int type, char *name, unsigned char *chipid);
int sunxi_get_power_key_irq(void);

int sunxi_get_voltage_by_phandle(const void *fdt, uint32_t phandle);
int sunxi_get_voltage_by_full_name(char *name);
int sunxi_set_voltage_by_phandle(const void *fdt, uint32_t phandle, uint vol_value, uint onoff);
int sunxi_set_voltage_by_full_name(char *name, uint vol_value, uint onoff);

bool sunxi_get_battery_exist(void);

bool sunxi_get_pmu_ext_get_exist(void);
bool sunxi_get_bmu_ext_get_exist(void);

int sunxi_bmu_info_update(void);
uint32_t sunxi_pmu_ext_info_update(void);

enum {
	SUNXI_POWER_PMU = 0,
	SUNXI_POWER_PMU_EXT,
	SUNXI_POWER_PMU_GENERAL,
	SUNXI_POWER_BMU,
	SUNXI_POWER_BMU_EXT,
	NR_SUNXI_POWER_TYPE_MAX,
};

/* legacy function */
extern int pmu_get_info(char *name, unsigned char *chipid);
extern int pmu_get_voltage(char *name);
extern int pmu_set_voltage(char *name, uint vol_value, uint onoff);
extern unsigned char pmu_get_reg_value(unsigned char reg_addr);
extern unsigned char pmu_set_reg_value(unsigned char reg_addr, unsigned char reg_value);

#ifdef CONFIG_SUNXI_PMU_EXT
extern int pmu_ext_get_type(void);
extern int pmu_ext_get_voltage(char *name);
extern int pmu_ext_set_voltage(char *name, uint vol_value, uint onoff);

extern const char *const pmu_ext_reg[];
extern const char *const pmu_ext[];
extern const int pmu_ext_type_max;
extern const int axp1530_ext_id;
#endif

#ifdef MACH_SUN8IW21
extern unsigned char pmu_axp2101_get_reg_value(unsigned char reg_addr);
extern unsigned char pmu_axp2101_set_reg_value(unsigned char reg_addr, unsigned char reg_value);
#endif

#endif
