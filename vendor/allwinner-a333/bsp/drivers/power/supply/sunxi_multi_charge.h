/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
#ifndef _SUNXI_MULTI_CHARGE_H_
#define _SUNXI_MULTI_CHARGE_H_

#include "sunxi-power-mfd.h"
#include "sunxi-power-notifier.h"
#include "sunxi-power-supply.h"

enum mc_input_detect_type {
	DETEC_UNKNOWN = 0,
	DETEC_BY_VBUS,
	DETEC_BY_TYPEC,
	DETEC_BY_GPIO,
	DETEC_BY_EXTCON,
	DETEC_TYPE_MAX,
};

struct mc_config_info {
	/* usb */
	u32 pmu_usbpc_cur;
	u32 pmu_usbad_cur;
};

struct mc_gpio_para {
	int	gpio;
	int irq_num;
};

#endif
