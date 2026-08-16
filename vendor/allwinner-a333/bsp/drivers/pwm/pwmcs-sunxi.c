/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinnertech pulse-width-modulation controller driver
 *
 * Copyright (C) 2015 AllWinner
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

/* #define DEBUG */
#define SUNXI_MODNAME "pwmcs"
#include <sunxi-log.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/pwm.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include "pwmcs-sunxi.h"
#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER
#include <linux/sunxi_amp_rsc.h>
#endif

#define PWMCS_NUM_MAX 8
#define PWM_BIND_NUM 2
#define PWM_PIN_STATE_ACTIVE "active"
#define PWM_PIN_STATE_SLEEP "sleep"
#define SUNXI_PWM_BIND_DEFAULT 255
#define SUNXI_PWM_PERIOD_DEFAULT 0
#define SUNXI_PWM_DUTY_DEFAULT 0
#define SUNXI_PWM_GROUP_CH_DEFAULT 0
#define SUNXI_PWM_GROUP_COUNT_DEFAULT 0


#define PRESCALE_MAX 256
#define SUNXI_PWM_NORMAL	1
#define SUNXI_PWM_INVERSED	0
#define SUNXI_PWM_SINGLE	1
#define SUNXI_PWM_DUAL		2
#define SUNXI_CLK_400M		400000000
#define SUNXI_CLK_100M		100000000
#define SUNXI_CLK_24M		24000000
#define SUNXI_DIV_CLK		1000000000

#define SUNXI_PWM_GROUP_PERIOD_MIN	10

#define SETMASK(width, shift)   ((width?((-1U) >> (32-width)):0)  << (shift))
#define CLRMASK(width, shift)   (~(SETMASK(width, shift)))
#define GET_BITS(shift, width, reg)     \
	    (((reg) & SETMASK(width, shift)) >> (shift))
#define SET_BITS(shift, width, reg, val) \
	    (((reg) & CLRMASK(width, shift)) | (val << (shift)))

//#define PWMCS_DEBUG
struct sunxi_pwm_config {
	unsigned int dead_time;
	unsigned int bind_pwm;
	unsigned int duty_cycle;
	unsigned int period;
};

struct group_pwm_config {
	unsigned int group_channel;
	unsigned int group_run_count;
	unsigned int pwm_polarity;
	int pwm_period;
};

struct sunxi_pwmcs_hw_data {
    /* pwmcs top register */
    u32 pcs_tier_offset;         /* PCS_TIER */
    u32 pcs_tisr_offset;         /* PCS TISR */
    u32 pcs_pgr0_offset;         /* PCS PGRO */
    u32 pcs_pgr1_offset;         /* PCS PGR1 */
    u32 pcs_pgr2_offset;         /* PCS PGR2 */
    u32 pcs_pgr3_offset;         /* PCS PGR3 */
    u32 pcs_cfg_data_offset;     /* PCS CFG DATA */
    u32 pcs_cfg_offset;          /* PCS CFG */
    u32 pcs_pwmcs_o_mult;        /* PCS PWMCS_O_MULT */
    u32 pcs_pwmcs_i_mult;        /* PCS PWMCS I MULT */
    u32 pcs_pwmcs_o_flt_e;       /* PCS PWMCS O FLT E */
    u32 pcs_pwmcs_iflten_offset; /* PCS PWMCS IF LTE N */
    u32 pcs_pwmcs_ofltv_offset;  /* PCS PWMCS OF LT V */
    u32 pcs_pwmcs_ifltv_offset;  /* PCS PWMCS IF LT V */
    u32 pcs_pwmcs_ilvl_de_offset;/* PCS PWMCS ILVL DE */
    u32 pcs_gcgr_offset;         /* PCS GCGR */
    u32 pcs_ver_offset;          /* PCS VER */

    /* pwmcs ch register */
    u32 pcs_pcier_offset;        /* PWMCS pwm channel irq enabled register */
    u32 pcs_pcisr_offset;        /* PWMCS pwm channel irq status register */
    u32 pcs_ccier_offset;        /* PWMCS capture channel irq enabled register */
    u32 pcs_ccisr_offset;        /* PWMCS capture channel irq status register */
    u32 pcs_cccier_offset;       /* PWMCS capture channel compare irq enabled register */
    u32 pcs_cccisr_offset;       /* PWMCS capture channel compare irq status register */
    u32 pcs_pos_irq_en_offset;   /* PCS POS IRQ EN */
    u32 pcs_pos_irq_sts_offset;  /* PCS POS IRQ STS */
    u32 pcs_pccr01_offset;       /* PCS PCCR01 */
    u32 pcs_pwm01outcon_offset;  /* PCS PWM01OUTCONF */
    u32 pcs_pcgr_offset;         /* PCS PCGR */
    u32 pcs_per_offset;          /* PCS PER */
    u32 pcs_pdzcr01_offset;      /* PCS PDZCR01 */
    u32 pcs_cccer_offset;        /* PCS CCCER */
    u32 pcs_cer_offset;          /* PCS CER */
    u32 pcs_cap01_ccr_offset;    /* PCS CAP01 CCR */
    u32 pcs_ccgr_offset;         /* PCS CCGR */
    u32 pcs_cap_pos_cnt_v_offset;/* PCS CAP POS CNT V */
    u32 pcs_cap_pos_cnt_sp_offset;/* PCS CAP POS CNT SP */
    u32 pcs_cap_pos_cnt_ep_offset;/* PCS CAP POS CNT EP */
    u32 pcs_cap_pos_cnt_c_offset; /* PCS CAP POS CNT C */
    u32 pcs_cap_pos_cnt_tm_cpl_capv_offset; /* PCS CAP POS CNT TM CPL CAPV */
    u32 cpl_cap_tmr_v_offset;    /* CPL CAP TMR V */
    u32 pcs_cap_tmr_prd_v_offset;/* PCS CAP TMR PRD V */
    u32 pcs_cap_dec_conf_offset; /* PCS CAP DEC CONF */
    u32 pcs_cap_poscnt_conf_offset;/* PCS CAP POSCNT CONF */
    u32 pcs_cap_tmr_cmp_conf_offset;/* PCS CAP TMR CMP CONF */
    u32 pcs_cap_rtcc_offset;     /* PCS CAP RTCC */
    u32 pcs_cap_rtcv_offset;     /* PCS CAP RTCV */
    u32 pcs_cap_rtc_dtdv_offset; /* PCS CAP RTC DTDV */
    u32 pcs_cap_rtc_dtv_offset;  /* PCS CAP RTC DTV */
    u32 pcs_cap_rtc_dtv_lh_offset;/* PCS CAP RTC DTV LH */
    u32 pcs_cap_pos_cnt_scapv_offset;/* PCS CAP POS CNT SCAPV */
    u32 pcs_cap_pos_cnt_ic_offset; /* PCS CAP POS CNT IC */
    u32 pcs_cap_pos_cnt_offset;  /* PCS CAP POS CNT */
    u32 pcs_cap_dir_err_va_offset;/* PCS CAP DIR ERR VA */
    u32 pcs_cap_modeo_a_p_offset;/* PCS CAP MODEO A P */
    u32 pcs_cap_modeo_ab_offset; /* PCS CAP MODEO AB */
    u32 pcs_cap_pos_cnt_cmp_lh_offset;/* PCS CAP POS CNT C */
    u32 pcs_pcr_base_offset;     /* PCS PCR base address */
    u32 pcs_pwmpul_num_offset;   /* PCS PWMPUL NUM offset */
    u32 pcs_pwm_entire_cycle_offset;/* PCS PWM ENTIRE CYCLE */
    u32 pcs_pwm_act_cycle_offset; /* PCS PWM ACT CYCLE */
    u32 pcs_ppcnt_cr_offset;     /* PCS PPCNT CR */
    u32 pcs_ppcnt_nm_sts_offset; /* PCS PPCNT NM STS */
    u32 pcs_ppcntr_offset;       /* PCS PPCNTR */
    u32 pcs_pwm_out_stp_offset;  /* PCS PWM OUT STP */
    u32 pcs_pwm_out_dly_offset;  /* PCS PWM OUT DLY */
    u32 pcs_pwm_cfg_fifo_sts_offset;/* PCS PWM CFG FIFO STS */
    u32 pcs_pwm_max_fentire_cycle_offset;/* PCS PWM MAX FENTIRE CYCLE */
    u32 pcs_pwm_min_fentire_cycle_offset;/* PCS PWM MIN FENTIRE CYCLE */
    u32 pcs_pwm_cnt_phv_offset;  /* PCS PWM CNT PHV */
    u32 pcs_ccr_offset;          /* PCS CCR */
    u32 pcs_crlr_offset;         /* PCS CRLR */
    u32 pcs_cflr_offset;         /* PCS CFLR */
    u32 pcs_cccsr_offset;        /* PCS CCCSR */
    u32 pcs_cccnr_offset;        /* PCS CCCNR */

    /* Additional configuration fields */
    u32 clk_gating_separate;     /* Whether clock gating is separate */
    u32 pwm_reg_uniform_offset;  /* Uniform offset between PWM registers */
    bool has_clock;              /* Whether the module has a clock */
    bool has_bus_clock;          /* Whether the module has a bus clock */
    bool has_hosc_clock;         /* Whether the module has a high-speed oscillator clock */
    bool config_status;          /* Configuration status */
};

struct sunxi_pwmcs_dma {
	struct dma_chan *chan;
	dma_addr_t dma_buf;
	unsigned int dma_len;
	enum dma_transfer_direction dma_transfer_dir;
	enum dma_data_direction dma_data_dir;
};

struct sunxi_pwmcs_chip {
	struct sunxi_pwmcs_hw_data *data;
	struct platform_device *pdev;
    struct device *dev;

	u32 *regs_backup;
	u32 *pm_regs_offset;
	int irq;
	int index;
	int pwm_num; /* the number of this PWM controller among all PWM controllers on this SoC */
	wait_queue_head_t wait;
	unsigned long cap_time[3];
	struct pwm_chip pwm_chip;
	struct resource *res;
	void __iomem *base;
	struct sunxi_pwm_config *config;
	struct group_pwm_config *group_config;
	struct clk *clk;
	struct clk *bclk;
	struct clk *hosc;
	struct reset_control *reset;
	unsigned int group_ch;
	unsigned int group_polarity;
	unsigned int group_period;
	struct pinctrl *pctl;
	unsigned int cells_num;
	bool status;
	bool use_dma;
	u32 dma_buf[32];
	struct sunxi_pwmcs_dma *dma_tx;
	struct sunxi_pwmcs_dma *dma_rx;
    unsigned int    len;
    u8 tx_fifosize;
    u8 tx_triglevel;
    u32 quirk_flag;
    struct sg_table tx_sg;
    size_t	max_dma_len;
    spinlock_t lock;
#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER
	sunxi_amp_rsc_t amp_rsc;
#endif
};

static struct sunxi_pwmcs_hw_data sunxi_pwmcs_v100_data = {
	/* pwmcs top register*/
	.pcs_tier_offset      = 0x0000,  // PCS_TIER
	.pcs_tisr_offset      = 0x0004,  // PCS TISR
	.pcs_pgr0_offset      = 0x0008,  // PCS PGRO
	.pcs_pgr1_offset      = 0x000C,  // PCS PGR1
	.pcs_pgr2_offset      = 0x0010,  // PCS PGR2
	.pcs_pgr3_offset      = 0x0014,  // PCS PGR3
	.pcs_cfg_data_offset  = 0x0018,  // PCS CFG DATA
	.pcs_cfg_offset       = 0x001C,  // PCS CFG
	.pcs_pwmcs_o_mult     = 0x0020,  // PCS PWMCS_O_MULT
	.pcs_pwmcs_i_mult     = 0x0024,  // PCS PWMCS I MULT
	.pcs_pwmcs_o_flt_e    = 0x0028,  // PCS PWMCS O FLT E
	.pcs_pwmcs_iflten_offset = 0x002C, //
	.pcs_pwmcs_ofltv_offset = 0x0030,
	.pcs_pwmcs_ifltv_offset = 0x0034,
	.pcs_pwmcs_ilvl_de_offset = 0x0038,
	.pcs_gcgr_offset = 0x003C,
	.pcs_ver_offset = 0x0040,

	/* pwmcs ch register*/
	.pcs_pcier_offset = 0x0000,
	.pcs_pcisr_offset = 0x0004,
	.pcs_ccier_offset = 0x0008,
	.pcs_ccisr_offset = 0x000C,
	.pcs_cccier_offset = 0x0010,
	.pcs_cccisr_offset = 0x0014,
	.pcs_pos_irq_en_offset = 0x0018,
	.pcs_pos_irq_sts_offset = 0x001C,
	.pcs_pccr01_offset = 0x0020,
	.pcs_pwm01outcon_offset = 0x0024,
	.pcs_pcgr_offset = 0x0028,
	.pcs_per_offset = 0x002C,
	.pcs_pdzcr01_offset = 0x0030,
	.pcs_cccer_offset = 0x0034,
	.pcs_cer_offset = 0x0038,
	.pcs_cap01_ccr_offset = 0x003c,
	.pcs_ccgr_offset = 0x0040,
	.pcs_cap_pos_cnt_v_offset = 0x0044,
	.pcs_cap_pos_cnt_sp_offset = 0x0048,
	.pcs_cap_pos_cnt_ep_offset = 0x004c,
	.pcs_cap_pos_cnt_c_offset = 0x0050,
	.pcs_cap_pos_cnt_tm_cpl_capv_offset = 0x0054,
	.cpl_cap_tmr_v_offset = 0x0058,
	.pcs_cap_tmr_prd_v_offset = 0x005C,
	.pcs_cap_dec_conf_offset = 0x0060,
	.pcs_cap_poscnt_conf_offset = 0x0064,
	.pcs_cap_tmr_cmp_conf_offset = 0x0068,
	.pcs_cap_rtcc_offset = 0x006C,
	.pcs_cap_rtcv_offset = 0x0070,
	.pcs_cap_rtc_dtdv_offset = 0x0074,
	.pcs_cap_rtc_dtv_offset = 0x0078,
	.pcs_cap_rtc_dtv_lh_offset = 0x007c,
	.pcs_cap_pos_cnt_scapv_offset = 0x0080,
	.pcs_cap_pos_cnt_ic_offset = 0x0084,
	.pcs_cap_pos_cnt_offset = 0x0088,
	.pcs_cap_dir_err_va_offset = 0x008C,
	.pcs_cap_modeo_a_p_offset = 0x0090,
	.pcs_cap_modeo_ab_offset = 0x0094,
	.pcs_cap_pos_cnt_cmp_lh_offset = 0x0098,
	.pcs_pcr_base_offset = 0x0100 + 0x0000,
	.pcs_pwmpul_num_offset = 0x0100 + 0x0004,
	.pcs_pwm_entire_cycle_offset = 0x0100 + 0x0008,
	.pcs_pwm_act_cycle_offset = 0x0100 + 0x000c,
	.pcs_ppcnt_cr_offset = 0x0100 + 0x0010,
	.pcs_ppcnt_nm_sts_offset = 0x0100 + 0x0014,
	.pcs_ppcntr_offset = 0x0100 + 0x0018,
	.pcs_pwm_out_stp_offset = 0x0100 + 0x001c,
	.pcs_pwm_out_dly_offset = 0x0100 + 0x0020,
	.pcs_pwm_cfg_fifo_sts_offset = 0x0100 + 0x0024,
	.pcs_pwm_max_fentire_cycle_offset = 0x0100 + 0x0028,
	.pcs_pwm_min_fentire_cycle_offset = 0x0100 + 0x002c,
	.pcs_pwm_cnt_phv_offset = 0x0100 + 0x0030,
	.pcs_ccr_offset = 0x0100 + 0x0034,
	.pcs_crlr_offset = 0x0100 + 0x0038,
	.pcs_cflr_offset = 0x0100 + 0x003c,
	.pcs_cccsr_offset = 0x0100 + 0x0040,
	.pcs_cccnr_offset = 0x0100 + 0x0044,
	.clk_gating_separate = 1,
	.pwm_reg_uniform_offset = 256,
	.has_clock = true,
	.has_bus_clock = true,
	.has_hosc_clock = true,
	.config_status = true,
};

static u32 sunxi_pwm_pre_scal[][2] = {
	/* reg_value  clk_pre_div */
	{0, 1},
	{1, 2},
	{2, 4},
	{3, 8},
	{4, 16},
	{5, 32},
	{6, 64},
	{7, 128},
	{8, 256},
};

int pwmcs_success_probe[8] = {0};

static inline struct sunxi_pwmcs_chip *to_sunxi_pwmcs_chip(struct pwm_chip *pwm_chip)
{
	return container_of(pwm_chip, struct sunxi_pwmcs_chip, pwm_chip);
}

static u32 sunxi_pwmcs_readl(struct pwm_chip *pwm_chip, u32 offset)
{
	u32 value;
	struct sunxi_pwmcs_chip *chip;

	chip = to_sunxi_pwmcs_chip(pwm_chip);

	value = readl(chip->base + offset);
	sunxi_debug(chip->pwm_chip.dev, "%3u bytes fifo\n", value);

	return value;
}

static u32 sunxi_pwmcs_writel(struct pwm_chip *pwm_chip, u32 offset, u32 value)
{
	struct sunxi_pwmcs_chip *chip;

	chip = to_sunxi_pwmcs_chip(pwm_chip);

	writel(value, chip->base + offset);

	return 0;
}

static int sunxi_pwmcs_pin_set_state(struct device *dev, char *name)
{
	struct pinctrl *pctl;
	struct pinctrl_state *state = NULL;
	int err;

	pctl = devm_pinctrl_get(dev);
	if (IS_ERR(pctl)) {
		sunxi_err(dev, "pinctrl_get failed\n");
		err = PTR_ERR(pctl);
		return err;
	}

	state = pinctrl_lookup_state(pctl, name);
	if (IS_ERR(state)) {
		sunxi_err(dev, "pinctrl_lookup_state(%s) failed\n", name);
		err = PTR_ERR(state);
		goto exit;
	}

	err = pinctrl_select_state(pctl, state);
	if (err) {
		sunxi_err(dev, "pinctrl_select_state(%s) failed\n", name);
		goto exit;
	}

exit:
	devm_pinctrl_put(pctl);
	return err;

}

static int sunxi_pwm_get_config(struct platform_device *pdev,
				struct sunxi_pwm_config *config, struct group_pwm_config *group_config)
{
	int err;
	struct device_node *np;

	np = pdev->dev.of_node;

	err = of_property_read_u32(np, "bind_pwm", &config->bind_pwm);
	if (err < 0) {
		/* if there is no bind pwm,set 255, dual pwm invalid! */
		config->bind_pwm = SUNXI_PWM_BIND_DEFAULT;
		err = 0;
	}

	err = of_property_read_u32(np, "dead_time", &config->dead_time);
	if (err < 0) {
		/* if there is bind pwm, but not set dead time,set bind pwm 255,dual pwm invalid! */
		config->bind_pwm = SUNXI_PWM_BIND_DEFAULT;
		err = 0;
	}

	err = of_property_read_u32(np, "group_channel", &group_config->group_channel);
	if (err < 0) {
		group_config->group_channel = SUNXI_PWM_GROUP_CH_DEFAULT;
		err = 0;
	}

	err = of_property_read_u32(np, "group_run_count", &group_config->group_run_count);
	if (err < 0) {
		group_config->group_run_count = SUNXI_PWM_GROUP_COUNT_DEFAULT;
		err = 0;
	}

	of_node_put(np);

	return err;
}

static int sunxi_pwm_set_polarity(struct pwm_chip *pwm_chip, struct pwm_device *pwm,
				enum pwm_polarity polarity)
{
	int bind_num, mode_num, i;
	u32 temp[2], index[2] = {0};
	struct sunxi_pwmcs_chip *chip;
	unsigned int reg_offset[2], reg_shift, reg_width;

	chip = to_sunxi_pwmcs_chip(pwm_chip);
	index[0] = pwm->hwpwm;
	reg_shift = PWMCS_ACT_STA_SHIFT;
	reg_width = PWMCS_ACT_STA_WIDTH;

	bind_num = chip->config[pwm->hwpwm].bind_pwm;
	if (bind_num == SUNXI_PWM_BIND_DEFAULT) {
		mode_num = SUNXI_PWM_SINGLE;
	} else {
		mode_num = SUNXI_PWM_DUAL;
		index[1] = bind_num - pwm_chip->base;
	}

	for (i = 0; i < mode_num; i++) {
		reg_offset[i] = PWMCS_CH_REG_ADDR(index[i], PCS_PCR) + PWMCS_CH_UNIFORM_OFFSET(index[i]);
		temp[i] = sunxi_pwmcs_readl(pwm_chip, reg_offset[i]);
	}

	/*
	 * config current pwm
	 * bind pwm's polarity is reverse compare with the current pwm
	 */

	spin_lock(&chip->lock);

	if (polarity == PWM_POLARITY_NORMAL)
		temp[0] = SET_BITS(reg_shift, reg_width, temp[0], SUNXI_PWM_NORMAL);
	else
		temp[0] = SET_BITS(reg_shift, reg_width, temp[0], SUNXI_PWM_INVERSED);

	if (mode_num == SUNXI_PWM_DUAL) {
		if (polarity == PWM_POLARITY_NORMAL) {
			temp[1] = SET_BITS(reg_shift, reg_width, temp[1], SUNXI_PWM_INVERSED);
		} else {
			temp[1] = SET_BITS(reg_shift, reg_width, temp[1], SUNXI_PWM_NORMAL);
		}
	}

	/* config register at the same time */
	for (i = 0; i < mode_num; i++)
		sunxi_pwmcs_writel(pwm_chip, reg_offset[i], temp[i]);

	spin_unlock(&chip->lock);

	return 0;
}

static ssize_t sunxi_pwm_group_period_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_pwmcs_chip *chip = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", chip->group_period);

}

static ssize_t sunxi_pwm_group_period_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	int err;
	u32 min;
	unsigned long val;
	struct sunxi_pwmcs_chip *chip = dev_get_drvdata(dev);

	min = SUNXI_PWM_GROUP_PERIOD_MIN;

	err = kstrtoul(buf, 10, &val);
	if (err)
		goto err_out;

	if (val < min)
		goto err_out;

	chip->group_period = val;

	return count;

err_out:
	sunxi_err(chip->pwm_chip.dev, "invalid parameter, group_polarity min val is %u!\n", min);

	return -EINVAL;
}

static ssize_t sunxi_pwm_group_polarity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sunxi_pwmcs_chip *chip = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", chip->group_polarity);

}

static ssize_t sunxi_pwm_group_polarity_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	int err;
	unsigned long val;
	struct sunxi_pwmcs_chip *chip = dev_get_drvdata(dev);

	err = kstrtoul(buf, 10, &val);
	if (err)
		goto err_out;

	if ((val != SUNXI_PWM_NORMAL) && (val != SUNXI_PWM_INVERSED))
		goto err_out;

	chip->group_polarity = val;

	return count;

err_out:
	sunxi_err(chip->pwm_chip.dev, "invalid parameter!\n");

	return -EINVAL;
}

static struct device_attribute sunxi_pwm_debug_attr[] = {
	__ATTR(group_period, S_IRUGO | S_IWUSR, sunxi_pwm_group_period_show, sunxi_pwm_group_period_store),
	__ATTR(group_polarity, S_IRUGO | S_IWUSR, sunxi_pwm_group_polarity_show, sunxi_pwm_group_polarity_store),
};

static void sunxi_pwm_create_sysfs(struct platform_device *pdev)
{
	u32 i;
	for (i = 0; i < ARRAY_SIZE(sunxi_pwm_debug_attr); i++)
		device_create_file(&pdev->dev, &sunxi_pwm_debug_attr[i]);
}

static void sunxi_pwm_remove_sysfs(struct platform_device *pdev)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(sunxi_pwm_debug_attr); i++)
		device_remove_file(&pdev->dev, &sunxi_pwm_debug_attr[i]);
}

static int sunxi_pwmcs_config_single(struct pwm_chip *pwm_chip, struct pwm_device *pwm_device,
		int duty_ns, int period_ns)
{
	u32 sel = 0;
	unsigned int reg_clk_src_shift, reg_clk_src_width;
	unsigned int reg_div_m_shift, reg_div_m_width;
	unsigned int reg_shift, reg_width;
	unsigned int reg_offset;
	unsigned int pre_scal_id = 0, div_m = 0, prescale = 0;
	unsigned long long clk_src = 0;
	unsigned long entire_cycles = 256, active_cycles = 192;
	unsigned int temp;
	struct sunxi_pwmcs_chip *chip;
	unsigned int value;

	chip = to_sunxi_pwmcs_chip(pwm_chip);
	sel = pwm_device->hwpwm;

	writel(0x1 << sel, chip->base + PWMCS_O_MULT_CFG);
	writel(0x6000, chip->base + PWMCS_CFG);

	reg_clk_src_shift = PWMCS_CLK_SRC_SHIFT;
	reg_clk_src_width = PWMCS_CLK_SRC_WIDTH;

	if (period_ns > 0 && period_ns <= 10) {
		clk_src = SUNXI_CLK_400M;

		reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCCR01);
		/* clk_src_reg */
		temp = readl(chip->base + reg_offset);
		temp = SET_BITS(reg_clk_src_shift, reg_clk_src_width, temp, 2); /* select clock source 400M */
		writel(temp, chip->base + reg_offset);

		reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCGR);
		temp = readl(chip->base + PCS_PCGR);
		temp = SET_BITS(0, 1, temp, 1); /* clk_gating set */
		temp = SET_BITS(16, 1, temp, 1); /* clk_bypass set */
		writel(temp, chip->base + reg_offset);
	} else if (period_ns > 10 && period_ns <= 334) {
		clk_src = SUNXI_CLK_100M;

		reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCCR01);
		temp = readl(chip->base + reg_offset);
		temp = SET_BITS(reg_clk_src_shift, reg_clk_src_width, temp, 1);
		writel(temp, chip->base + reg_offset);
	} else if (period_ns > 334) {
		/* if freq < 3M, then select 24M clock */
		clk_src = SUNXI_CLK_24M;

		reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCCR01);
		/* clk_src_reg : use OSC24M clock */
		temp = readl(chip->base + reg_offset);
		temp = SET_BITS(reg_clk_src_shift, reg_clk_src_width, temp, 0);
		writel(temp, chip->base + reg_offset);
	}

	clk_src = clk_src * period_ns;
	do_div(clk_src, SUNXI_DIV_CLK);
	entire_cycles = (unsigned long)clk_src;  /* How many clksrc beats in a PWM period */

	/* get entire cycle length */
	for (pre_scal_id = 0; pre_scal_id < 9; pre_scal_id++) {
		if (entire_cycles <= 65536)
			break;
		for (prescale = 0; prescale < PRESCALE_MAX+1; prescale++) {
			entire_cycles = ((unsigned long)clk_src/sunxi_pwm_pre_scal[pre_scal_id][1])/(prescale + 1);
			if (entire_cycles <= 65536) {
				div_m = sunxi_pwm_pre_scal[pre_scal_id][0];
				break;
			}
		}
	}

	clk_src = (unsigned long long)entire_cycles * duty_ns;
	do_div(clk_src, period_ns);
	active_cycles = clk_src;
	if (entire_cycles == 0)
		entire_cycles++;

	/* config clk div_m */
	reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCCR01);
	reg_div_m_shift = PWMCS_DIV_M_SHIFT;
	reg_div_m_width = PWMCS_DIV_M_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_div_m_shift, reg_div_m_width, temp, div_m);
	writel(temp, chip->base + reg_offset);

	if (!chip->config[sel].period && !chip->config[sel].duty_cycle) {
		/* config gating */
		reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCGR);
		reg_shift = sel;
		value = readl(chip->base + reg_offset);
		value = SET_BITS(reg_shift, 1, value, 1); /* set gating */
		writel(value, chip->base + reg_offset);
	}

	/* config prescal */
	reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCR) + PWMCS_CH_UNIFORM_OFFSET(sel);
	reg_shift = PWMCS_PRESCAL_SHIFT;
	reg_width = PWMCS_PRESCAL_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, prescale);
	writel(temp, chip->base + reg_offset);

	reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCR) + PWMCS_CH_UNIFORM_OFFSET(sel);
	reg_shift = PWMCS_CFG_MODE_SHIFT;
	reg_width = PWMCS_CFG_MODE_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, 1);
	writel(temp, chip->base + reg_offset);

	/* config active cycles */
	reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PWM_ACT_CYCLE) + PWMCS_CH_UNIFORM_OFFSET(sel);
	reg_shift = PWMCS_ACT_CYCLES_SHIFT;
	reg_width = PWMCS_ACT_CYCLES_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, active_cycles);
	writel(temp, chip->base + reg_offset);

	/* config entire cycles */
	reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PWM_ENTIRE_CY) + PWMCS_CH_UNIFORM_OFFSET(sel);
	reg_shift = PWMCS_ENTIRE_CYCLES_SHIFT;
	reg_width = PWMCS_ENTIRE_CYCLES_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, (entire_cycles - 1));
	writel(temp, chip->base + reg_offset);

	if ((chip->config[sel].period && (chip->config[sel].period != period_ns)) ||
		(chip->config[sel].duty_cycle && chip->config[sel].duty_cycle != duty_ns)) {
		reg_offset = PWMCS_CH_REG_ADDR(sel, PCS_PCR) + PWMCS_CH_UNIFORM_OFFSET(sel);
		reg_shift = PWMCS_NEW_DATA_SHIFT;
		reg_width = PWMCS_NEW_DATA_WIDTH;
		temp = readl(chip->base + reg_offset);
		temp = SET_BITS(reg_shift, reg_width, temp, 1);
		writel(temp, chip->base + reg_offset);
	}

	if (chip->config[sel].period != period_ns)
		chip->config[sel].period = period_ns;

	if (chip->config[sel].duty_cycle != duty_ns)
		chip->config[sel].duty_cycle = duty_ns;

	return 0;
}

static int sunxi_pwm_config_channel(struct pwm_chip *pwm_chip, struct pwm_device *pwm_device,
		int duty_ns, int period_ns)
{
	int ret;

	struct sunxi_pwmcs_chip *chip;

	chip = to_sunxi_pwmcs_chip(pwm_chip);

	ret = sunxi_pwmcs_config_single(pwm_chip, pwm_device, duty_ns, period_ns);

	return ret;
}

static int sunxi_pwmcs_resource_get(struct platform_device *pdev,
				struct sunxi_pwmcs_chip *chip,
				struct device_node *np)
{
	struct resource *res;
	int err, i;
	const char *st = NULL;
	struct platform_device *pwm_pdevice;
	struct device_node *sub_np;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		sunxi_err(&pdev->dev, "fail to get pwm IORESOURCE_MEM\n");
		return -EINVAL;
	}

	chip->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(chip->base)) {
		sunxi_err(&pdev->dev, "fail to map pwm IO resource\n");
		return PTR_ERR(chip->base);
	}

	chip->pwm_num = of_alias_get_id(np, "pwmcs");
	if (chip->pwm_num < 0) {
		sunxi_err(&pdev->dev, "failed to get alias id\n");
		return -EINVAL;
	}

	sunxi_info(&pdev->dev, "chip->pwmcs_num:%d\n", chip->pwm_num);

	if (chip->pwm_num != 0) {
		for (i = 0; i < chip->pwm_num; i++) {
			if (pwmcs_success_probe[i] == 0) {
				sunxi_err(&pdev->dev, "pwmcs%d must probe after all of pwmcs0 ~ pwmcs%d probed, now pwmcs%d not probe",
						chip->pwm_num, chip->pwm_num -1, i);
				return -EPROBE_DEFER;
			}
		}
	}

	/*
	 * If there are clock resources, has_clock is true, apply for clock resources;
	 * otherwise, has_clock is false, you do not need to apply for clock resources.
	 */

	if (chip->data->has_clock) {
		chip->reset = devm_reset_control_get_optional(&pdev->dev, NULL);
		if (IS_ERR(chip->reset)) {
			sunxi_err(&pdev->dev, "can't get pwm reset clk\n");
			return PTR_ERR(chip->reset);
		}

		chip->clk = devm_clk_get(&pdev->dev, "mod");
		if (!chip->clk) {
			chip->clk = of_clk_get(pdev->dev.of_node, 0);
			if (IS_ERR_OR_NULL(chip->clk)) {
				sunxi_err(&pdev->dev, "fail to get pwm clk!\n");
				return -EINVAL;
			}
		}

		if (chip->data->has_bus_clock) {
			chip->bclk = devm_clk_get(&pdev->dev, "clk_bus_pwm");
			if (!chip->bclk) {
				sunxi_err(&pdev->dev, "fail to get pwm clk!\n");
				return -EINVAL;
			}
		}

		if (chip->data->has_hosc_clock) {
			chip->hosc = devm_clk_get(&pdev->dev, "clk_hosc");
			if (!chip->hosc) {
				sunxi_err(&pdev->dev, "fail to get clk_hosc!\n");
				return -EINVAL;
			}
		}
	}

	/* read property pwm-number */
	err = of_property_read_u32(np, "pwm-number", &chip->pwm_chip.npwm);
	if (err) {
		sunxi_err(&pdev->dev, "failed to get pwm number!\n");
		return -EINVAL;
	}

	/* read property pwm-base */
	err = of_property_read_u32(np, "pwm-base", &chip->pwm_chip.base);
	if (err) {
		sunxi_err(&pdev->dev, "failed to get pwm-base!\n");
		return -EINVAL;
	}

	sunxi_err(&pdev->dev, "base is %d, num is %d\n", chip->pwm_chip.base, chip->pwm_chip.npwm);

	err = of_property_read_u32(np, "#pwm-cells", &chip->cells_num);
	if (err) {
		sunxi_err(&pdev->dev, "failed to get pwm-cells!\n");
		return -EINVAL;
	}

	if (chip->data->config_status) {
		err = of_property_read_string(np, "status", &st);
		if (err) {
			sunxi_err(&pdev->dev, "failed to get status!\n");
			return -EINVAL;
		}
		if (st && (!strcmp(st, "okay") || !strcmp(st, "ok")))
			chip->status = true;
	}

	chip->config = devm_kzalloc(&pdev->dev, sizeof(*chip->config) * chip->pwm_chip.npwm, GFP_KERNEL);
	if (!chip->config)
		return -ENOMEM;

	chip->group_config = devm_kzalloc(&pdev->dev, sizeof(*chip->group_config) * chip->pwm_chip.npwm, GFP_KERNEL);
	if (!chip->group_config)
		return -ENOMEM;

	for (i = 0; i < chip->pwm_chip.npwm; i++) {
		/* set all the pwm channel to singal mode as dafault */
		chip->config[i].bind_pwm = SUNXI_PWM_BIND_DEFAULT;
		chip->config[i].duty_cycle = SUNXI_PWM_DUTY_DEFAULT;
		chip->config[i].period = SUNXI_PWM_PERIOD_DEFAULT;
		chip->group_config[i].group_channel = SUNXI_PWM_GROUP_CH_DEFAULT;

		sub_np = of_parse_phandle(np, "sunxi-pwms", i);
		if (!sub_np) {
			sunxi_err(&pdev->dev, "can't parse \"sunxi-pwms\" property\n");
			return -EINVAL;
		}

		pwm_pdevice = of_find_device_by_node(sub_np);
		/* it may be the program is error or the status of pwm%d  is disabled */
		if (!pwm_pdevice) {
			sunxi_debug(&pdev->dev, "fail to find device for pwm%d, continue!\n", i);
			continue;
		}

		err = sunxi_pwm_get_config(pwm_pdevice, &chip->config[i], &chip->group_config[i]);
		if (err) {
			sunxi_err(&pdev->dev, "Get config failed,exit!\n");
			return err;
		}
	}

	return 0;
}

static void sunxi_pwm_resource_put(struct sunxi_pwmcs_chip *chip)
{

}

static int sunxi_pwm_clk_enable(struct sunxi_pwmcs_chip *chip)
{
	int err;

	if (chip->data->has_clock) {
		/*
		 * In order to ensure the consistent display from uboot to the kernel stage,
		 * there is no need to reset the clock in the kernel stage.
		 */
		err = reset_control_deassert(chip->reset);
		if (err) {
			sunxi_err(chip->pwm_chip.dev, "deassert pwm reset failed\n");
			return err;
		}

		if (chip->data->has_hosc_clock) {
			err = clk_prepare_enable(chip->hosc);
			if (err) {
				sunxi_err(chip->pwm_chip.dev, "try to enbale pwm hosc clk failed\n");
				return err;
			}

			err = clk_set_parent(chip->clk, chip->hosc);
			if (err) {
				sunxi_err(chip->pwm_chip.dev, "clk_set_parent() failed!\n");
				return err;
			}
		}

		err = clk_prepare_enable(chip->clk);
		if (err) {
			sunxi_err(chip->pwm_chip.dev, "try to enbale pwm clk failed\n");
			reset_control_assert(chip->reset);
			return err;
		}

		if (chip->data->has_bus_clock) {
			err = clk_prepare_enable(chip->bclk);
			if (err) {
				sunxi_err(chip->pwm_chip.dev, "try to enbale pwm bclk failed\n");
				clk_disable_unprepare(chip->clk);
				reset_control_assert(chip->reset);
				return err;
			}
		}
	}

	return 0;
}

static void sunxi_pwm_clk_disable(struct sunxi_pwmcs_chip *chip)
{
	if (chip->data->has_clock) {
		if (chip->data->has_bus_clock)
			clk_disable_unprepare(chip->bclk);
		clk_disable_unprepare(chip->clk);
		reset_control_assert(chip->reset);
	}
}

static irqreturn_t sunxi_pwm_handler(int irq, void *dev_id)
{
	pr_err("TODO: sunxi pwm handler\n");
	return IRQ_HANDLED;
}

static int sunxi_pwm_hw_init(struct platform_device *pdev,
			     struct sunxi_pwmcs_chip *chip)
{
	int err;

	err = sunxi_pwm_clk_enable(chip);
	if (err) {
		sunxi_err(&pdev->dev, "enable pwm clock failed\n");
		return err;
	}

	chip->irq = platform_get_irq(pdev, 0);
	if (chip->irq < 0)
		sunxi_info(&pdev->dev, "get interrupt resource failed and capture mode invalid\n");
	else {
		init_waitqueue_head(&chip->wait);
		err = devm_request_irq(&pdev->dev, chip->irq, sunxi_pwm_handler, IRQF_TRIGGER_NONE, "pwmcs", chip);
		if (err) {
			sunxi_err(&pdev->dev, "failed to request PWM IRQ\n");
			sunxi_pwm_clk_disable(chip);
			return err;
		}
	}

	return 0;
}

static void sunxi_pwm_hw_exit(struct sunxi_pwmcs_chip *chip)
{
	sunxi_pwm_clk_disable(chip);
}

#ifdef PWMCS_DEBUG
static void sunxi_pwmcs_dma_callback(void *arg)
{
	struct sunxi_pwmcs_chip *chip = (struct sunxi_pwmcs_chip *)arg;

	if (chip->dma_tx) {
		sunxi_err(chip->pwm_chip.dev, "drv-mode: dma write data end\n");
	} else {
		sunxi_err(chip->pwm_chip.dev, "drv-mode: dma read data end\n");
	}
}

static int sunxi_pwmcs_request_dma(struct sunxi_pwmcs_chip *chip, struct sunxi_pwmcs_dma **_info, const char *name)
{
	int ret = 0;

	*_info = kzalloc(sizeof(**_info), GFP_KERNEL);
	if (IS_ERR_OR_NULL(*_info)) {
		sunxi_err(chip->pwm_chip.dev, "can't kzalloc dma info\n");
		return -ENOMEM;
	}

	(*_info)->chan = dma_request_chan(chip->pwm_chip.dev, name);
	if (IS_ERR_OR_NULL((*_info)->chan)) {
		ret = PTR_ERR((*_info)->chan);
		sunxi_err(chip->pwm_chip.dev, "can't request DMA tx channel %d\n", ret);
		return PTR_ERR((*_info)->chan);
	}

	return 0;
}

static void sunxi_pwmcs_dma_release(struct sunxi_pwmcs_chip *chip)
{
	if (chip->dma_tx) {
		chip->dma_tx->dma_buf = 0;
		chip->dma_tx->dma_len = 0;
		dma_release_channel(chip->dma_tx->chan);
		chip->dma_tx->chan = NULL;
		kfree(chip->dma_tx);
		chip->dma_tx = NULL;
	}
}

static int sunxi_pwmcs_dma_deinit(struct sunxi_pwmcs_chip *chip, struct sunxi_pwmcs_dma **_info)
{
	struct device *chan_dev = (*_info)->chan->device->dev;

	dma_unmap_single(chan_dev, (*_info)->dma_buf, (*_info)->dma_len, (*_info)->dma_data_dir);

	return 0;
}

static int sunxi_pwmcs_dma_init(struct sunxi_pwmcs_chip *chip, struct sunxi_pwmcs_dma **_info)
{
	struct dma_slave_config dma_sconfig;
	struct device *chan_dev;
	struct dma_async_tx_descriptor *dma_desc;
	dma_addr_t phy_addr;
	int err;

	phy_addr = (dma_addr_t)chip->res->start;

	dma_sconfig.dst_addr = phy_addr + PWMCS_CFG_DATA;
	dma_sconfig.direction = DMA_MEM_TO_DEV;
	(*_info)->dma_transfer_dir = DMA_MEM_TO_DEV;
	(*_info)->dma_data_dir = DMA_TO_DEVICE;
	(*_info)->dma_len = 128;

	dma_sconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_sconfig.src_maxburst = 4;
	dma_sconfig.dst_maxburst = 4;

	err = dmaengine_slave_config((*_info)->chan, &dma_sconfig);
	if (err < 0) {
		sunxi_err(chip->pwm_chip.dev, "can't configure tx channel\n");
		goto err0;
	}

	chan_dev = (*_info)->chan->device->dev;

	/* kzalloc buf to store the dma xfered data */
	// sunxi_debug(chip->pwm_chip.dev, "%d\n",__LINE__);
	// sunxi_debug(chip->pwm_chip.dev, "chip->dma_buf:0x%x\n",chip->dma_buf);
	// sunxi_debug(chip->pwm_chip.dev, "(*_info)->dma_len:%d\n",(*_info)->dma_len);
	// sunxi_debug(chip->pwm_chip.dev, "(*_info)->dma_len:%d\n",(*_info)->dma_len);
	// sunxi_debug(chip->pwm_chip.dev, "(*_info)->dma_data_dir:%d\n",(*_info)->dma_data_dir);

	(*_info)->dma_buf = dma_map_single(chan_dev, chip->dma_buf,
					(*_info)->dma_len, (*_info)->dma_data_dir);
	if (dma_mapping_error(chan_dev, (*_info)->dma_buf)) {
		sunxi_err(chip->pwm_chip.dev, "DMA mapping failed\n");
		err = -EINVAL;
		goto err0;
	}

	dma_desc = dmaengine_prep_slave_single((*_info)->chan, (*_info)->dma_buf,
					       (*_info)->dma_len, (*_info)->dma_transfer_dir,
					       DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!dma_desc) {
		sunxi_err(chip->pwm_chip.dev, "Not able to get desc for DMA xfer\n");
		err = -EINVAL;
		goto err2;
	}

	// sunxi_debug(chip->pwm_chip.dev, "(*_info)->dma_buf:%px\n",(*_info)->dma_buf);
	// sunxi_debug(chip->pwm_chip.dev, "dma_desc:%px\n",dma_desc);

	dma_desc->callback = sunxi_pwmcs_dma_callback;
	dma_desc->callback_param = chip;
	err = dma_submit_error(dmaengine_submit(dma_desc));
	if (err) {
		sunxi_err(chip->pwm_chip.dev, "DMA submit failed!\n");
		err = -EINVAL;
		goto err2;
	}
	dmaengine_submit(dma_desc);

	return 0;
err2:
	sunxi_pwmcs_dma_deinit(chip, _info);
err0:
	return err;
}

/* set dma start flag, if queue, it will auto restart to transfer next queue */
static void sunxi_pwm_dma_start(struct sunxi_pwmcs_chip *chip, struct sunxi_pwmcs_dma **_info)
{
	dma_async_issue_pending((*_info)->chan);
	sunxi_info(chip->pwm_chip.dev, "DMA start!\n");
}

static int sunxi_pwmcs_dma_output_config(struct sunxi_pwmcs_chip *chip)
{
	sunxi_debug(chip->pwm_chip.dev, "TODO: sunxi pwmcs dma_output config\n");

	return 0;
}

static int sunxi_pwmcs_dma_tx_config(struct sunxi_pwmcs_chip *chip)
{
	int err;

	if (!chip->dma_tx) {
		err = sunxi_pwmcs_request_dma(chip, &chip->dma_tx, "tx");
		if (err) {
			sunxi_err(chip->pwm_chip.dev, "request dma_tx failed\n");
			goto err0;
		}
	}

	err = sunxi_pwmcs_dma_init(chip, &chip->dma_tx);
	if (err) {
		sunxi_err(chip->pwm_chip.dev, "dma_tx xfer init failed\n");
		goto err1;
	}

	sunxi_pwmcs_dma_output_config(chip);

	sunxi_pwm_dma_start(chip, &chip->dma_tx);

	return 0;
err1:
	sunxi_pwmcs_dma_release(chip);
err0:
	chip->dma_tx = NULL;
	return err;
}
#endif

static int sunxi_pwmcs_enable_single(struct pwm_chip *pwm_chip, struct pwm_device *pwm_device)
{
	unsigned int index = 0;
	unsigned int reg_offset, reg_shift, reg_width;
	unsigned int temp;
	unsigned long flags;
	struct sunxi_pwmcs_chip *chip;
	struct platform_device *pwm_pdevice;
	struct device_node *sub_np;
	int err;

	chip = to_sunxi_pwmcs_chip(pwm_chip);

	index = pwm_device->hwpwm;
	sunxi_debug(pwm_chip->dev, "pwm chan %d is enable\n", index);

	sub_np = of_parse_phandle(pwm_chip->dev->of_node, "sunxi-pwms", index);
	if (!sub_np) {
		sunxi_err(chip->pwm_chip.dev, "can't parse \"sunxi-pwms\" property\n");
		return -ENODEV;
	}
	pwm_pdevice = of_find_device_by_node(sub_np);
	if (!pwm_pdevice) {
		sunxi_err(chip->pwm_chip.dev, "can't parse pwm device\n");
		return -ENODEV;
	}

	err = sunxi_pwmcs_pin_set_state(&pwm_pdevice->dev, PWM_PIN_STATE_ACTIVE);
	if (err != 0)
		return err;

	spin_lock_irqsave(&chip->lock, flags);
	/* pwm channel enable */
	reg_offset = PWMCS_CH_REG_ADDR(index, PCS_PER);
	reg_shift = PWMCS_OUT_EN_SHIFT;
	reg_width = PWMCS_OUT_EN_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, 1);
	writel(temp, chip->base + reg_offset);

	reg_offset = PWMCS_CH_REG_ADDR(index, PCS_PCR) + PWMCS_CH_UNIFORM_OFFSET(index);
	reg_shift = PWMCS_CFG_MODE_SHIFT;
	reg_width = PWMCS_CFG_MODE_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, 1);
	writel(temp, chip->base + reg_offset);

	reg_shift = PWMCS_NEW_DATA_SHIFT;
	reg_width = PWMCS_NEW_DATA_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, 1);
	writel(temp, chip->base + reg_offset);

	reg_shift = PWMCS_UPDATE_MODE_SHIFT;
	reg_width = PWMCS_UPDATE_MODE_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, 1);
	writel(temp, chip->base + reg_offset);

	spin_unlock_irqrestore(&chip->lock, flags);

	return 0;
}

static void sunxi_pwmcs_disable_single(struct pwm_chip *pwm_chip, struct pwm_device *pwm_device)
{
	unsigned int index = 0;
	unsigned int reg_offset, reg_shift, reg_width;
	unsigned int temp;
	unsigned long flags;
	struct sunxi_pwmcs_chip *chip;
	struct device_node *sub_np;
	struct platform_device *pwm_pdevice;

	chip = to_sunxi_pwmcs_chip(pwm_chip);

	sub_np = of_parse_phandle(pwm_chip->dev->of_node, "sunxi-pwms", index);
	if (IS_ERR_OR_NULL(sub_np)) {
		sunxi_err(chip->pwm_chip.dev, "can't parse \"sunxi-pwms\" property\n");
		return;
	}
	pwm_pdevice = of_find_device_by_node(sub_np);
	if (IS_ERR_OR_NULL(pwm_pdevice)) {
		sunxi_err(chip->pwm_chip.dev, "can't parse pwm device\n");
		return;
	}

	index = pwm_device->hwpwm;
	sunxi_debug(pwm_chip->dev, "pwm chan %d is enable\n", index);

	spin_lock_irqsave(&chip->lock, flags);
	/* disable pwm channel*/
	reg_offset = PWMCS_CH_REG_ADDR(index, PCS_PER);
	reg_shift = PWMCS_OUT_EN_SHIFT;
	reg_width = PWMCS_OUT_EN_WIDTH;
	temp = readl(chip->base + reg_offset);
	temp = SET_BITS(reg_shift, reg_width, temp, 0);
	writel(temp, chip->base + reg_offset);
	spin_unlock_irqrestore(&chip->lock, flags);

	sunxi_pwmcs_pin_set_state(&pwm_pdevice->dev, PWM_PIN_STATE_SLEEP);
}

static int sunxi_pwm_enable(struct pwm_chip *pwm_chip, struct pwm_device *pwm_device)
{
	int ret;
	struct sunxi_pwmcs_chip *chip;

	chip = to_sunxi_pwmcs_chip(pwm_chip);

	ret = sunxi_pwmcs_enable_single(pwm_chip, pwm_device);

	return ret;
}

static void sunxi_pwm_disable(struct pwm_chip *pwm_chip, struct pwm_device *pwm_device)
{
	int bind_num;
	struct sunxi_pwmcs_chip *chip;

	chip = to_sunxi_pwmcs_chip(pwm_chip);

	bind_num = chip->config[pwm_device->hwpwm].bind_pwm;

	sunxi_pwmcs_disable_single(pwm_chip, pwm_device);
}

static struct pwm_ops sunxi_pwm_ops = {
	.config = sunxi_pwm_config_channel,
	.enable = sunxi_pwm_enable,
	.disable = sunxi_pwm_disable,
	.set_polarity = sunxi_pwm_set_polarity,
	.owner = THIS_MODULE,
};

static const struct of_device_id sunxi_pwmcs_match[] = {
	{ .compatible = "allwinner,sunxi-pwmcs-v100", .data = &sunxi_pwmcs_v100_data},
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, sunxi_pwmcs_match);

static int sunxi_pwmcs_fill_hw_data(struct sunxi_pwmcs_chip *chip)
{
	const struct of_device_id *of_id;

	/* get hw data from match table */
	of_id = of_match_device(sunxi_pwmcs_match, chip->pwm_chip.dev);
	if (!of_id) {
		sunxi_err(chip->pwm_chip.dev, "of_match_device() failed\n");
		return -EINVAL;
	}

	chip->data = (struct sunxi_pwmcs_hw_data *)(of_id->data);

	return 0;
}

static int sunxi_pwmcs_probe(struct platform_device *pdev)
{
	int ret;
	struct sunxi_pwmcs_chip *chip;
	struct device_node *np = pdev->dev.of_node;

	sunxi_info(&pdev->dev, "start probe");

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	platform_set_drvdata(pdev, chip);
	chip->pwm_chip.dev = &pdev->dev;
	chip->pdev = pdev;

#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER
	ret = sunxi_pdev_request_peri_rsc(chip->pdev, "pwmcs_drv", &chip->amp_rsc);
	if (ret) {
		sunxi_err(&pdev->dev, "request AMP system peri resource for pwm failed, ret: %d\n", ret);
		return ret;
	}
#endif

	ret = sunxi_pwmcs_fill_hw_data(chip);
	if (ret) {
		sunxi_err(&pdev->dev, "unable to get hw_data\n");
		return ret;
	}

	ret = sunxi_pwmcs_resource_get(pdev, chip, np);
	if (ret) {
		sunxi_err(&pdev->dev, "pwm failed to get resource\n");
		goto err0;
	}

	if (chip->data->config_status && !chip->status) {
		sunxi_debug(&pdev->dev, "the current status of pwmchip is disabled, it should not be loaded\n");
		goto err1;
	}

	ret = sunxi_pwm_hw_init(pdev, chip);
	if (ret) {
		sunxi_err(&pdev->dev, "pwm failed to hw_init");
		goto err1;
	}

	chip->pwm_chip.dev = &pdev->dev;
	chip->pwm_chip.ops = &sunxi_pwm_ops;
	chip->pwm_chip.of_xlate = of_pwm_xlate_with_flags;
	chip->pwm_chip.of_pwm_n_cells = chip->cells_num;

	spin_lock_init(&chip->lock);

	/*
	 * register pwm chip to pwm-core should be the ending of probe
	 * before registering, all pwm controller resources need to be ready
	 * (pwm_request can happen anytime after registration)
	 */
	ret = pwmchip_add(&chip->pwm_chip);
	if (ret < 0) {
		sunxi_err(&pdev->dev, "register pwmchip failed: %d\n", ret);
		goto err2;
	}

	sunxi_pwm_create_sysfs(chip->pdev);

	pwmcs_success_probe[chip->pwm_num] = 1;
	sunxi_info(&pdev->dev, "pwmcschip probe success\n");

	return 0;

err2:
	sunxi_pwm_hw_exit(chip);
err1:
	sunxi_pwm_resource_put(chip);
err0:
	return ret;
}

static int sunxi_pwmcs_remove(struct platform_device *pdev)
{
	struct sunxi_pwmcs_chip *chip;
#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER
	int err;
#endif

	chip = platform_get_drvdata(pdev);

	pwmchip_remove(&chip->pwm_chip);
	sunxi_pwm_remove_sysfs(chip->pdev);
	sunxi_pwm_hw_exit(chip);

#ifdef CONFIG_AW_AMP_SYS_RSC_MANAGER
	err = sunxi_amp_rsc_free(chip->amp_rsc);
	if (err)
		sunxi_err(chip->pwm_chip.dev, "release AMP system resource for pwm failed, ret: %d\n", err);
#endif

	sunxi_pwm_resource_put(chip);

	return 0;
}

static const struct dev_pm_ops pwmcs_pm_ops;

static struct platform_driver sunxi_pwmcs_driver = {
	.probe = sunxi_pwmcs_probe,
	.remove = sunxi_pwmcs_remove,
	.driver = {
		.name = "sunxi_pwmcs",
		.owner  = THIS_MODULE,
		.of_match_table = sunxi_pwmcs_match,
		.pm = &pwmcs_pm_ops,
	 },
};

static int __init pwmcs_module_init(void)
{
	return platform_driver_register(&sunxi_pwmcs_driver);
}

static void __exit pwmcs_module_exit(void)
{
	platform_driver_unregister(&sunxi_pwmcs_driver);
}

subsys_initcall_sync(pwmcs_module_init);
module_exit(pwmcs_module_exit);

MODULE_AUTHOR("zhangyunhui");
MODULE_DESCRIPTION("pwmcs driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-pwmcs");
MODULE_VERSION("1.0.1");
