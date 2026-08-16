// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Allwinner Technology Co., Ltd.
 *
 * Written by: fenglizhen <fenglizhen@allwinnertech.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/compat.h>


/* CCMU */
#define CCMU_BASE			(0x03008000)
#define CCMU_AHB_MAT_CLK_GATE_EN_REG	(CCMU_BASE+0x05C0)                 // AHB Master Clock Gate Enable Register
#define CCMU_USB0_CLK_REG		(CCMU_BASE+0x1300)                 // USB0 Clock Register
#define CCMU_USB0_GAR_REG		(CCMU_BASE+0x1304)                 // USB0 Gating And Reset Register
#define CCMU_USB1_CLK_REG		(CCMU_BASE+0x1308)                 // USB1 Clock Register
#define CCMU_USB1_GAR_REG		(CCMU_BASE+0x130C)                 // USB1 Gating And Reset Register
#define CCMU_USB2P0_SYS_PHY_REF_CLK_REG	(CCMU_BASE+0x1340)                 // USB2P0_SYS PHY Reference Clock Register
#define CCMU_USB2P0_SYS_GAR_REG		(CCMU_BASE+0x1344)                 // USB2P0_SYS Gating And Reset Register
#define CCMU_USB2_U2_PHY_REF_CLK_REG	(CCMU_BASE+0x1348)                 // USB2_U2_REF PHY Reference Clock Register
#define CCMU_USB2_SUSPEND_CLK_REG	(CCMU_BASE+0x1350)                 // USB2 SUSPEND Clock Register
#define CCMU_USB2_MF_CLK_REG		(CCMU_BASE+0x1354)                 // USB2 MF Clock Register
#define CCMU_USB2_GAR_REG		(CCMU_BASE+0x135C)                 // USB2 Gating And Reset Register
#define CCMU_USB2_U3_ONLY_UTMI_CLK_REG	(CCMU_BASE+0x1360)                 // USB2_U3_ONLY_UTMI Clock Register
#define CCMU_USB2_U2_ONLY_PIPE_CLK_REG	(CCMU_BASE+0x1364)                 // USB2_U2_ONLY_PIPE Clock Register

// USB31 Bus Gating Reset Register
#define CCMU_HSI_COMB0_PHY_CFG_CLK_REG	(CCMU_BASE+0x13C0)                 // HSI COMB0 PHY Configure Clock Register
#define CCMU_HSI_COMB0_PHY_REF_CLK_REG	(CCMU_BASE+0x13C4)                 // HSI COMB0 PHY Reference Clock Register
#define CCMU_HSI_SYS_GAR_REG		(CCMU_BASE+0x13CC)                 // HSI_SYS  Gating And Reset Register
#define CCMU_HSI_AXI_CLK_REG		(CCMU_BASE+0x13E0)                 // HSI AXI Clock Register
#define CCMU_RES24M_GATE_EN_REG		(CCMU_BASE+0x1A00)                 // RES 24M Gate Enable Register
#define CCMU_CM_HSI_CFG_REG		(CCMU_BASE+0x1B28)                 // CM USB3 Enable Configuration Register

/* USB3 Controller */
#define USB3_OTG_BASE			(0x0C400000)
#define USB3_OTG_USB2_ISCR		(USB3_OTG_BASE + 0x100000)
#define USB3_OTG_USB2_PHYCTL		(USB3_OTG_BASE + 0x100010) // decrepted
#define USB3_OTG_USB2_PHYTST		(USB3_OTG_BASE + 0x100014)
#define USB3_OTG_USB2_PHYTUNE		(USB3_OTG_BASE + 0x100018)
#define USB3_OTG_USB2_PHYSTS		(USB3_OTG_BASE + 0x100024)

/* Hsi Sub-System Application */
#define HSI_SYS_APP_BASE		(0x0DC80000) // 512K

/* Hsi Comb0 PHY Top Application */
#define HSI_COMB0_PHY_TOP_BASE		(0x0DD00000) // 512K

/* USB2P0 PHY Application */
#define USB2P0_PHY_APP_BASE		(0x0C000000) // 512K
#define USB2P0_PHY0_USB_PHY_CTRL	(USB2P0_PHY_APP_BASE + 0x0010)
#define USB2P0_PHY1_USB_PHY_CTRL	(USB2P0_PHY_APP_BASE + 0x0030)
#define USB2P0_PHY2_USB_PHY_CTRL	(USB2P0_PHY_APP_BASE + 0x0050)
#define USB2P0_PHY0_USB_RST_CTRL	(USB2P0_PHY_APP_BASE + 0x0028)
#define USB2P0_PHY1_USB_RST_CTRL	(USB2P0_PHY_APP_BASE + 0x0048)
#define USB2P0_PHY2_USB_RST_CTRL	(USB2P0_PHY_APP_BASE + 0x0068)


void aw_delay(u32 n)
{
	while (n--) {
		// delay...
	};
}

static void usb3_system_config(void)
{
	unsigned int val;
	// 1.CCMU 相关配置(CCMU寄存器)
	// 1.1.USB2 U2 Ref Clock Gating配置, CCMU 0x1348 USB2_U2_
	val = readl(CCMU_USB2_U2_PHY_REF_CLK_REG);
	val |= (0x01 << 31);
	writel(val, CCMU_USB2_U2_PHY_REF_CLK_REG); //24M

	// 1.2.USB2 Suspend clock相关配置, CCMU 0x1350 USB2_SUSPE
	val = readl(CCMU_USB2_SUSPEND_CLK_REG);
	val |= (0x01 << 31);
	val |= (0x01 << 24);
	writel(val, CCMU_USB2_SUSPEND_CLK_REG); //32k

	// 1.3.HSI COMB0 PHY cfg clock相关配置, CCMU 0x13C0 HSI_C
	val = readl(CCMU_HSI_COMB0_PHY_CFG_CLK_REG);
	val |= (0x01 << 31);
	val |= (0x01 << 24);
	val |= (0x05);
	writel(val, CCMU_HSI_COMB0_PHY_CFG_CLK_REG);

	// 1.4.HSI COMB0 PHY Ref Clock相关配置, CCMU 0x13C4 HSI_C
	val = readl(CCMU_HSI_COMB0_PHY_REF_CLK_REG);
	val |= (0x01 << 31);
	writel(val, CCMU_HSI_COMB0_PHY_REF_CLK_REG);

	// 1.5.AHB Gate相关配置, CCMU 0x05C0 AHB_MAT_CLK_GATE_EN_
	val = readl(CCMU_AHB_MAT_CLK_GATE_EN_REG);
	// val |= (0x01 << 16);
	val |= (0x01 << 11);
	// val |= (0x01 << 4);
	writel(val, CCMU_AHB_MAT_CLK_GATE_EN_REG);

	// 1.6.RES 24M Gate相关配置, CCMU 0x1A00 RES24M_GATE_EN_R
	val = readl(CCMU_RES24M_GATE_EN_REG);
	// val |= (0x01 << 3);
	val |= (0x01 << 0);
	writel(val, CCMU_RES24M_GATE_EN_REG);

	// 1.7.MBUS Master Clock Gate相关配置, CCMU 0x05E4 bit[25

	// 1.8.HSI COMB0 PHY Ref Clock相关配置, CCMU 0x13C4 HSI_C
	val = readl(CCMU_HSI_COMB0_PHY_REF_CLK_REG);
	val |= (0x01 << 31);
	writel(val, CCMU_HSI_COMB0_PHY_REF_CLK_REG);

	// 1.9.HSI Bus Gate Reset相关配置 CCMU 0x13CC HSI_SYS_GAR
	val = readl(CCMU_HSI_SYS_GAR_REG);
	// val |= (0x01 << 17);
	val |= (0x01 << 16);
	val |= (0x01 << 1);
	val |= (0x01 << 0);
	writel(val, CCMU_HSI_SYS_GAR_REG);

	// 1.10.HSI AXI Clock GATE相关配置 CCMU 0x13E0 HSI_AXI_CL
	val = readl(CCMU_HSI_AXI_CLK_REG);
	val |= (1 << 31);
	val |= (1 << 24);
	writel(val, CCMU_HSI_AXI_CLK_REG);

	// 2.HSI_COMB0_PHY复位配置(HSI_SYS寄存器)
	// 2.1.HSI_COMB0_PHY复位释放 HSI_COMB0_PHY_CTL 0x0000 bit

	// 3.USB2_U2 aclk/hclk_en utmi_clk/pipe_cll选择(HSI_SYS寄
	// 3.1.USB_BGR寄存器 0x0008 bit16/17配置为1, bit20配置为1
	val = readl(HSI_SYS_APP_BASE + 0x0008);
	val &= ~(1 << 21);
	val |= (1 << 20);
	val |= (1 << 17);
	val |= (1 << 16);
	writel(val, HSI_SYS_APP_BASE + 0x0008);

	// 4.USB0_USB1 PHY MAPPING配置(HSI_SYS寄存器)
	// 4.1 USB2_U2_MAC_MAP寄存器 保持默认配置即可 切换mapping
	// 4.2 USB2_U2_PHY_MAP寄存器 保持默认配置即可 切换mapping

	// 5. USB2P0_SYS时钟复位配置(CCMU寄存器)
	// 5.1.USB2P0_SYS时钟Gating和复位配置 CCMU 0x1344 USB2P0_
	val = readl(CCMU_USB2P0_SYS_GAR_REG);
	val |= (0x01 << 16);
	val |= (0x01 << 0);
	writel(val, CCMU_USB2P0_SYS_GAR_REG);

	// 5.2.USB2_U2_ONLY_PIPE CLK配置 CCMU 0x1364 USB2_U2_ONLY
	val = readl(CCMU_USB2_U2_ONLY_PIPE_CLK_REG);
	val |= (0x01 << 31);
	val |= (0x01 << 0);
	writel(val, CCMU_USB2_U2_ONLY_PIPE_CLK_REG);

	// 6.USB2P0 PHY 复位和SIDDQ配置(USB2寄存器)
	// 6.1.复位配置 USB_RST_CTRL寄存器 0x0C00_0028,0x0C00_004
	val = readl(USB2P0_PHY0_USB_RST_CTRL);
	val |= (0x01 << 0);
	writel(val, USB2P0_PHY0_USB_RST_CTRL);

	val = readl(USB2P0_PHY1_USB_RST_CTRL);
	val |= (0x01 << 0);
	writel(val, USB2P0_PHY1_USB_RST_CTRL);

	val = readl(USB2P0_PHY2_USB_RST_CTRL);
	val |= (0x01 << 0);
	writel(val, USB2P0_PHY2_USB_RST_CTRL);

	// 6.2.SIDDQ配置 USB_PHY_CTRL寄存器 0x0C00_0010,0x0C00_00
	val = readl(USB2P0_PHY0_USB_PHY_CTRL);
	val &= ~(0x01 << 3);
	writel(val, USB2P0_PHY0_USB_PHY_CTRL);

	val = readl(USB2P0_PHY1_USB_PHY_CTRL);
	val &= ~(0x01 << 3);
	writel(val, USB2P0_PHY1_USB_PHY_CTRL);

	val = readl(USB2P0_PHY2_USB_PHY_CTRL);
	val &= ~(0x01 << 3);
	writel(val, USB2P0_PHY2_USB_PHY_CTRL);

	// 7.CCMU时钟复位配置(CCMU寄存器)
	// 7.1.USB2复位配置 CCMU 0x135C USB2_GAR_REG bit16[0 --- no] 配置为1
	val = readl(CCMU_USB2_GAR_REG);
	val |= (0x01 << 16);
	// val |= (0x01 << 0);
	writel(val, CCMU_USB2_GAR_REG);

	// 7.2.USB2_MF_CLK Gate配置 CCMU 0x1354 USB2_MF_CLK_REG b
	val = readl(CCMU_USB2_MF_CLK_REG);
	val |= (0x01 << 31);
	writel(val, CCMU_USB2_MF_CLK_REG);

	// 7.3.USB2_U3_ONLY_UTMI_CLK gate配置 CCMU 0x1360 USB2_U3
	val = readl(CCMU_USB2_U3_ONLY_UTMI_CLK_REG);
	val |= (0x01 << 31);
	writel(val, CCMU_USB2_U3_ONLY_UTMI_CLK_REG);

	// 8.USB2_U2 aclk/hclk_en utmi_clk/pipe_cll选择(HSI_SYS寄
	// 8.1.USB_BGR寄存器 0x0008 bit16/17配置为1 bit20配置为1,
	val = readl(HSI_SYS_APP_BASE + 0x0008);
	val &= ~(1 << 21);
	val |= (1 << 20);
	val |= (1 << 17);
	val |= (1 << 16);
	writel(val, HSI_SYS_APP_BASE + 0x0008);

	// 9.USB2_U2_PHY_MODE配置(HSI_SYS寄存器)
	// 9.1.USB2_U2_PHY_MODE寄存器 0x000C bit0配置为1
	val = readl(HSI_SYS_APP_BASE + 0x000C);
	val |= (1 << 0);
	writel(val, HSI_SYS_APP_BASE + 0x000C);

	// 10.USB3.1 Gen1 Controller App配置(USB3.1 APP寄存器)
	// 10.1.强制Vbus有效 USB2_ISCR 0x0000 bit13/12配置为1
	val = readl(USB3_OTG_USB2_ISCR);
	val |= (0x01 << 13);
	val |= (0x01 << 12);
	writel(val, USB3_OTG_USB2_ISCR);

	// 10.2.强制ID为高 USB2_ISCR 0x0000 bit15/14配置为1
	// 10.3.配置SIDDQ使能PHY USB_PHY_CTL 0x0008 bit3配置为0
}

static void usb3_close_all(void)
{
	// USB3_U2 PHY SIDDQ close
	// subsystem usb3 bus gatting close
}

void sunxi_usb_phy_init(void)
{
	usb3_system_config();
	printf("sunxi USB3 init!\n");
}

void sunxi_usb_phy_deinit(void)
{
	usb3_close_all();
	printf("sunxi USB3 deinit!\n");
}