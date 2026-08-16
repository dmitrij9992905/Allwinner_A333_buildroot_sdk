/* SPDX-License-Identifier: GPL-2.0+ */
#include <asm/arch/plat-sun50iw15p1/cpu_autogen.h>

#define SUNXI_GIC400_BASE             (SUNXI_CPU_GIC400_BASE)
#define SUNXI_PIO_BASE                (SUNXI_GPIO_BASE)
#define SUNXI_R_PIO_BASE              (SUNXI_R_GPIO_BASE)
#define SUNXI_RTC_DATA_BASE           (SUNXI_RTC_BASE+0x100)
/*#define SUNXI_RSB_BASE*/
#define SUNXI_CCM_BASE                (SUNXI_CCMU_BASE)
#define SUNXI_EHCI1_BASE              (SUNXI_USB0_BASE + 0x1000)
#define SUNXI_USBOTG_BASE             (SUNXI_USB0_BASE)
#define SUNXI_DMA_BASE                (SUNXI_DMAC_BASE)
#define SUNXI_MMC0_BASE               (SUNXI_SMHC0_BASE)
#define SUNXI_MMC1_BASE               (SUNXI_SMHC1_BASE)
#define SUNXI_MMC2_BASE               (SUNXI_SMHC2_BASE)
#define SUNXI_PRCM_BASE               (SUNXI_R_PRCM_BASE)
#define SUNXI_SS_BASE                 (SUNXI_CE_SYS_BASE)
#define SUNXI_WDT_BASE                (0x02051000)

/* timer */
#define	 SUNXI_TIMER_BASE	SUNXI_TIMER0_BASE

#define SUNXI_CPU_SYS_CFG_BASE 		  (SUNXI_CPUX_PLL_CFG_BASE_BASE)
#define	SUNXI_SYSCTRL_BASE	SUNXI_SYS_CFG_BASE

#define R_PRCM_REG_BASE               (0x07010000)
#define SUNXI_RTWI_BRG_REG            (SUNXI_PRCM_BASE+0x019c)
#define SUNXI_R_UART_BASE             (0x07080000)
#define SUNXI_R_TWI_BASE              (0x07081400)

#define PIOC_REG_o_POW_MOD_SEL 0x340
#define PIOC_REG_o_POW_MS_CTL 0x350
#define PIOC_REG_o_POW_MS_VAL 0x348

#define PIOC_REG_POW_MOD_SEL (SUNXI_PIO_BASE + PIOC_REG_o_POW_MOD_SEL)
#define PIOC_REG_POW_MS_CTL (SUNXI_PIO_BASE + PIOC_REG_o_POW_MS_CTL)
#define PIOC_REG_POW_VAL (SUNXI_PIO_BASE + PIOC_REG_o_POW_MS_VAL)

#define PIOC_SEL_Px_3_3V_VOL 1
#define PIOC_SEL_Px_1_8V_VOL 0

#define PIOC_CTL_Px_ENABLE 0
#define PIOC_CTL_Px_DISABLE 1

#define PIOC_VAL_Px_3_3V_VOL 0
#define PIOC_VAL_Px_1_8V_VOL 1

#define PIOC_CTL_Px_DEFUALT PIOC_CTL_Px_ENABLE
#define PIOC_SEL_Px_DEFAULT PIOC_SEL_Px_1_8V_VOL

#define SUNXI_TIMESTEMP_COUNTER_REG	(SUNXI_CPUX_TIMESTAMP_STA_BASE)
