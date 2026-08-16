/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
*/

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/timer.h>
#include "private_uboot.h"

// don't need in fpga
#if 0
static void setbit(u32 cpux, u8 bit);
static void clearbit(u32 cpux, u8 bit);
#endif

void clock_init_uart(void)
{
	return;
}

// don't need in fpga
#if 0
static void setbit(u32 cpux, u8 bit)
{
	u32 reg_val;
	reg_val = readl(cpux);
	reg_val |= (1 << bit);
	writel(reg_val, cpux);
}

static void clearbit(u32 cpux, u8 bit)
{
	u32 reg_val;
	reg_val = readl(cpux);
	reg_val &= ~(1 << bit);
	writel(reg_val, cpux);
}

static void enable_pll(u32 cpux, struct core_pll_freq_tbl *pll)
{

}
#endif

int clock_set_corepll(int frequency)
{
	return  0;
}


uint clock_get_ddrpll(void)
{
	return 0;
}

uint clock_get_corepll(void)
{

	return 0;
}


uint clock_get_axi(void)
{

	return 0;
}


uint clock_get_ahb(void)
{

	return 0;
}


uint clock_get_apb1(void)
{

	return 0;
}

uint clock_get_apb2(void)
{

	return 0;
}


uint clock_get_mbus(void)
{
	return 0;
}

int sunxi_set_sramc_mode(void)  // ???
{
	u32 reg_val;

	/* SRAM Area C 128K Bytes Configuration by AHB ,default map to VE*/
	reg_val = readl(SUNXI_SYSCTRL_BASE);
	reg_val &= ~(0xFFFFFFFF);
	writel(reg_val, SUNXI_SYSCTRL_BASE);

	/* VE SRAM:set sram to normal mode, default boot mode */
	reg_val = readl(SUNXI_SYSCTRL_BASE + 0X0004);
	reg_val &= ~(0x1 << 24);
	writel(reg_val, SUNXI_SYSCTRL_BASE + 0X0004);

	return 0;
}

int sunxi_get_active_boot0_id(void) // ??
{
	uint32_t val = *(uint32_t *)(SUNXI_RTC_BASE + 0x304);
	if (val & (1 << 15)) {
		return (val >> 8) & 0x7;
	} else {
		return (val >> 24) & 0x7;
	}
}

void clock_open_timer(int timernum)   // todo -> song
{
	u32 reg_value = 0;

	reg_value = readl(SUNXI_CCM_BASE + 0x730 + timernum * 4);
	reg_value |= (0 <<24);
	reg_value |= (2 << 0);
	writel(reg_value, (SUNXI_CCM_BASE + 0x730 + timernum * 4));

	/*enable timer*/
	reg_value = readl(SUNXI_CCM_BASE + 0x730 + timernum * 4);
	reg_value |= (1 << 31);
	writel(reg_value, (SUNXI_CCM_BASE + 0x730 + timernum * 4));

	reg_value = readl(SUNXI_CCM_BASE + 0x74c);
	reg_value |= (1 << 16);
	writel(reg_value, (SUNXI_CCM_BASE + 0x74c));

	reg_value = readl(SUNXI_CCM_BASE + 0x74c);
	reg_value |= (1 << 0);
	writel(reg_value, (SUNXI_CCM_BASE + 0x74c));
	__msdelay(1);

}

void clock_close_timer(int timernum)   // todo -> song
{
	u32 reg_value = 0;

	/*disable timer*/
	reg_value = readl(SUNXI_CCM_BASE + 0x730 + timernum * 4);
	reg_value |= (0 << 31);
	writel(reg_value, (SUNXI_CCM_BASE + 0x730 + timernum * 4));

	reg_value = readl(SUNXI_CCM_BASE + 0x74c);
	reg_value |= (0 << 16);
	writel(reg_value, (SUNXI_CCM_BASE + 0x74c));

	reg_value = readl(SUNXI_CCM_BASE + 0x74c);
	reg_value |= (0 << 0);
	writel(reg_value, (SUNXI_CCM_BASE + 0x74c));
	__msdelay(1);
}

void clock_set_gic(void)  // todo -> song
{
	u32 reg_value = 0;

	reg_value = readl(SUNXI_CCM_BASE + GIC_CLK_REG);
	reg_value |= ((1 << 31) | (2 << 24) | (0 << 0));
	writel(reg_value, SUNXI_CCM_BASE + GIC_CLK_REG);
	udelay(20);
}
