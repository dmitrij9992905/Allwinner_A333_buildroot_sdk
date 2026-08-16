
/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Liaoyongming <liaoyongming@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 */

#include <sunxi_board.h>
#include <spare_head.h>
#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <securestorage.h>
#include <asm/arch/rtc.h>
#include <environment.h>

#ifdef CONFIG_SUNXI_SBI_ECALL
#include <sbi.h>
#else
#include <smc.h>
#endif

#define	FDT_PATH_BOX_START	"/box_start_os0"

typedef enum start_mode {
	COLD_START = 0x0,
	FAKE_POWEROFF = 0x2,
	FAKE_POWEROFF_E = 0xe,	/* compatible with old platform, such as sun50iw9 */
	BOOT_NORMAL = 0xf
} start_mode_e;

typedef enum start_type {
	START2FAKE_POWEROFF = 0x0,
	START2NORMAL_BOOT = 0x1,
	START2MEM = 0x2
} start_type_e;

static void sunxi_fake_poweroff(void)
{
#ifdef CONFIG_SUNXI_SBI_ECALL
	sbi_ecall_box_standby();
#else
	arm_svc_fake_poweroff((ulong)working_fdt);
#endif
}

/* parse cec enable param and then deliver to cpus when enter
 * poweroff/fake-poweroff
 *
 * The param parsed will decide cec function/cec wakeup function is
 * enable or not.
 * Function will be disable by default if env is not set.
 *
 * return:
 * 0
 */
int parse_platform_private_data(void)
{
	char *cec_enable;
	char *cec_wakeup_enable;
	int cec_en = 0;
	int cec_wakeup_en = 0;

	/* judge if CEC_ENABLE/CEC_WAKUP_ENABLE key have set by android */
	cec_enable = env_get("CEC_ENABLE");
	cec_wakeup_enable = env_get("CEC_WAKUP_ENABLE");

	pr_msg("[cec_enable]:%s, [cec_wakeup_enable]:%s\n", cec_enable, cec_wakeup_enable);

	if (cec_enable == NULL)
		pr_err("env[CEC_ENABLE] is not exist!\n");
	else {
		if (!strncmp(cec_enable, "1", sizeof("1")))
			cec_en = 1;

		if (cec_wakeup_enable == NULL)
			pr_err("env[CEC_WAKE_ENABLE] is not exist!\n");
		else {
			if (!strncmp(cec_wakeup_enable, "1", sizeof("1")))
				cec_wakeup_en = 1;
		}

		rtc_write_data(CONFIG_FAKE_POWEROFF_PRI_DATA_INDEX, (cec_en << 1) | cec_wakeup_en);
	}

	return 0;
}

/* parse startup type from factoty menu
 *
 * The result decide homlet/TV startup in fake poweroff
 * or startup direcely.
 *
 * return:
 * COLD_START,
 * FAKE_POWEROFF,
 * BOOT_NORMAL
 */
start_mode_e parse_factory_menu(void)
{
	char standby_char[10] = "standby";
	char direct_char[10] = "direct";
	char memory_char[10] = "memory";
	char *boot_mode;
	char *mem_mode;

	/* judge if factory menu has set standby flag */
	boot_mode = env_get("BOOTMODE");
	if (boot_mode == NULL) {
		pr_err("env[BOOTMODE] has no start mode flag\n");
		return COLD_START;
	} else {
		pr_msg("env[BOOTMODE]:%s\n", boot_mode);
		if (!strncmp(boot_mode, standby_char, strlen(standby_char)))
			return FAKE_POWEROFF;

		if (!strncmp(boot_mode, direct_char, strlen(direct_char)))
			return BOOT_NORMAL;

		if (!strncmp(boot_mode, memory_char, strlen(memory_char))) {
			mem_mode = env_get("MEM_MODE");
			if (mem_mode == NULL) {
				pr_err("env[MEM_MODE] has no mem mode flag\n");
				return COLD_START;
			} else {
				pr_msg("env[MEM_MODE]:%s\n", mem_mode);
				if (!strncmp(mem_mode, standby_char, strlen(standby_char)))
					return FAKE_POWEROFF;

				if (!strncmp(mem_mode, direct_char, strlen(direct_char)))
					return BOOT_NORMAL;
			}
		}
	}
	return COLD_START;
}

/* parse defaule startup type from dts node 'start_type'
 *
 * The result decide homlet/TV startup in fake poweroff
 * or startup direcely.
 *
 * return:
 * COLD_START,
 * FAKE_POWEROFF,
 * BOOT_NORMAL
 */
start_mode_e parse_default_start_mode(void)
{
	int nodeoffset = -1;
	start_type_e default_mode;
	char standby_char[10] = "standby";
	char direct_char[10] = "direct";
	char memory_char[10] = "memory";
	char *mem_mode;

	nodeoffset = fdt_path_offset(working_fdt, FDT_PATH_BOX_START);
	if (!nodeoffset) {
		pr_err("default start_type is not found!\n");
		return COLD_START;
	}

	fdt_getprop_u32(working_fdt, nodeoffset, "start_type", &default_mode);
	mem_mode = env_get("BOOTMODE");
	if (mem_mode == NULL) {
		switch (default_mode) {
		case START2FAKE_POWEROFF:
			env_set("BOOTMODE", standby_char);
			pr_err("env[BOOTMODE] rewrite %s to env\n", standby_char);
			env_save();
			return FAKE_POWEROFF;
		case START2MEM:
			env_set("BOOTMODE", memory_char);
			pr_err("env[BOOTMODE] rewrite %s to env\n", memory_char);
			env_save();
			return BOOT_NORMAL;
		case START2NORMAL_BOOT:
		default:
			env_set("BOOTMODE", direct_char);
			pr_err("env[BOOTMODE] rewrite %s to env\n", direct_char);
			env_save();
			return BOOT_NORMAL;
		}
	}
	return BOOT_NORMAL;
}

/*
 * parse_box_standby
 *
 * return:
 * 0 - normal boot
 * 1 - maybe box standby
 */
#if defined(CONFIG_MACH_SUN50IW9) || defined(CONFIG_MACH_SUN50IW12)
static int parse_box_standby(void)
{
	return 1;
}
#else
static uint32_t __maybe_unused read_start_mode(void)
{
	return rtc_read_data(CONFIG_FAKE_POWEROFF_FLAG_INDEX);
}

static void __maybe_unused write_start_mode(uint32_t mode)
{
	rtc_write_data(CONFIG_FAKE_POWEROFF_FLAG_INDEX, mode);
}

static int parse_box_standby(void)
{
	int ret = 0;
	start_mode_e start_mode;

	/* get start_mode from FAKE_POWEROFF_FLAG rtc reg */
	start_mode = read_start_mode();

	pr_force("start_mode: %x\n", start_mode);

	switch (start_mode) {
	case FAKE_POWEROFF:
	case FAKE_POWEROFF_E:
		{
			pr_force("enter fake poweroff\n");
			ret = 1;
			break;
		}
	case COLD_START:
		{
#ifdef CONFIG_SUNXI_BOX_STANDBY_FACTORY_MENU
			start_mode_e menu_mode;
			start_mode_e default_mode;

			menu_mode = parse_factory_menu();
			default_mode = parse_default_start_mode();

			switch (menu_mode) {
			case FAKE_POWEROFF:
				{
					ret = 1;
					break;
				}
			case BOOT_NORMAL:
				{
					ret = 0;
					break;
				}
			default:
				switch (default_mode) {
				case FAKE_POWEROFF:
					{
						ret = 1;
						break;
					}
				case BOOT_NORMAL:
				default:
					{
						ret = 0;
						break;
					}
				}
			}
			break;
#else
			int node;
			start_type_e start_type = START2NORMAL_BOOT;

			node = fdt_path_offset(working_fdt, FDT_PATH_BOX_START);
			if (node < 0) {
				return 0;
			}

			/* get start_type from dts config*/
			fdt_getprop_u32(working_fdt, node, "start_type", &start_type);

			pr_force("start_type: %x\n", start_type);

			if (start_type == START2FAKE_POWEROFF)
				ret = 1;
			else if (start_type == START2NORMAL_BOOT)
				ret = 0;
			else
				ret = 0;
			break;
#endif
		}
	case BOOT_NORMAL:
	default:
		{
			ret = 0;
			break;
		}
	}

	return ret;
}
#endif

int sunxi_box_standby(void)
{

	if (get_boot_work_mode() != WORK_MODE_BOOT) {
		return 0;
	}

	if (!parse_box_standby())
		return 0;

	parse_platform_private_data();

	sunxi_fake_poweroff();
	return 0;
}
