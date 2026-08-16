/*
 * Allwinner Sun60iw2 do poweroff in uboot with arisc
 *
 * (C) Copyright 2021  <xinouyang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <sunxi_board.h>
#include <smc.h>
#include <asm/arch/usb.h>
#include <asm/arch/watchdog.h>

#ifdef CONFIG_SUNXI_POWER
#include <sunxi_power/sunxi_power.h>
#include <fdt_support.h>
#endif
#include <efuse_map.h>

int sunxi_platform_power_off(int status)
{
	int work_mode = get_boot_work_mode();
	/* imporve later */
	ulong cfg_base = 0;

	if (work_mode != WORK_MODE_BOOT)
		return 0;

	/* startup cpus before shutdown */
	arm_svc_arisc_startup(cfg_base);

	if (status)
		arm_svc_poweroff_charge();
	else
		arm_svc_poweroff();

	while (1) {
		asm volatile ("wfi");
	}
	return 0;
}

#ifdef CONFIG_SUNXI_PMU_EXT
int update_pmu_ext_info_to_kernel(void)
{
	int nodeoffset, err;
	uint32_t phandle = 0;

	phandle = sunxi_pmu_ext_info_update();

	/* get cpu@4 node */
	nodeoffset = fdt_path_offset(working_fdt, "cpu-ext");
	if (nodeoffset < 0) {
		pr_err("## error: %s: L%d\n", __func__, __LINE__);
		return -1;
	}

	/* Change cpu-supply to ext dcdc*/
	err = fdt_setprop_u32(working_fdt, nodeoffset,
				"cpu-supply", phandle);
	if (err < 0) {
		pr_warn("WARNING: fdt_setprop can't set %s from node %s: %s\n",
			"compatible", "status", fdt_strerror(err));
		return -1;
	}

	return 0;
}
#endif

#define SRAM_CTRL_REG2 (SUNXI_SYSCTRL_BASE + 0x8)
int sunxi_set_sramc_mode(void)
{
	u32 reg_val;

	/* SRAM:set sram to npu, default boot mode */
	reg_val = readl(SRAM_CTRL_REG2);
	reg_val &= ~(0x1 << 1);
	writel(reg_val, SRAM_CTRL_REG2);
	debug("set sram to npu\n");

	reg_val = readl(SRAM_CTRL_REG2);
	if (reg_val & (0x1 << 1))
		pr_err("set sram to npu fail!\n");
	return 0;
}

int sunxi_get_active_boot0_id(void)
{
	uint32_t val = *(uint32_t *)(SUNXI_RTC_BASE + 0x318);
	if (val & (1 << 15)) {
		return (val >> 12) & 0x7;
	} else {
		return (val >> 28) & 0x7;
	}
}

void otg_phy_config(void)
{
	u32 reg_val;
	reg_val = readl((const volatile void __iomem *)(SUNXI_USBOTG_BASE +
							USBC_REG_o_PHYCTL));
	reg_val &= ~(0x01 << USBC_PHY_CTL_SIDDQ);
	reg_val |= 0x01 << USBC_PHY_CTL_VBUSVLDEXT;
	writel(reg_val, (volatile void __iomem *)(SUNXI_USBOTG_BASE +
						  USBC_REG_o_PHYCTL));
}

void sunxi_board_reset_cpu(ulong addr)
{
	static const struct sunxi_wdog *wdog =
		(struct sunxi_wdog *)SUNXI_WDT_BASE;

	writel(((WDT_CFG_KEY << 16) | 0x02), &wdog->ocfg);
	/* Set the watchdog for its shortest interval (.5s) and wait */
	writel(((WDT_CFG_KEY << 16) | WDT_MODE_EN), &wdog->srst);
	while (1) { }

}

#ifdef CONFIG_SUNXI_POWER
int update_bmu_info_to_kernel(void)
{
	sunxi_bmu_info_update();

	return 0;
}
#endif

void sunxi_sid_enable_checksum(void)
{
#ifdef EFUSE_CONFIG
	uint reg_val = 0;

	reg_val  = sid_read_key(EFUSE_CONFIG);
	if (reg_val & (0x1 << EFUSE_CHECK_SUM_OFFSET)) {
		//printf("Has enable checksum, brom config checksum: %x\n", reg_val);
		return;
	} else {
		reg_val |= (0x01 << EFUSE_CHECK_SUM_OFFSET);
		sid_program_key(EFUSE_CONFIG, reg_val);
		reg_val = (sid_read_key(EFUSE_CONFIG) >> EFUSE_CHECK_SUM_OFFSET) & 1;

		printf("burn enable checksum, brom config checksum: %x\n", reg_val);
	}
#endif
	return;
}


