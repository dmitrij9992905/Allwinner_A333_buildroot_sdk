/* SPDX-License-Identifier: GPL-2.0 */
/*
 * sunxi's Remote Processor Share Interrupt Head File
 *
 * Copyright (C) 2022 Allwinnertech - All Rights Reserved
 *
 * Author: lijiajian <lijiajian@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <dt-bindings/gpio/sun8iw22-share-irq-dt.h>

static struct share_irq_res_table share_irq_table[] = {
	/* major  host coreq hwirq  remore core irq */
	/* table for e907 */
	{1,    ARM_PA_IRQ_NUM},
	{2,    ARM_PB_IRQ_NUM},
	{3,    ARM_PC_IRQ_NUM},
	{4,    ARM_PD_IRQ_NUM},
	{5,    ARM_PE_IRQ_NUM},
	{6,    ARM_PF_IRQ_NUM},
	{7,    ARM_PG_IRQ_NUM},
	{10,   ARM_PJ_IRQ_NUM},
	{11,   ARM_PK_IRQ_NUM},
};
