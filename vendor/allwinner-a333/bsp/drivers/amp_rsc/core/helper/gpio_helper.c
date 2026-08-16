// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the People's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY'S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS'SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY'S TECHNOLOGY.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
 * PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
 * THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
 * OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../asrm_core.h"

#include "helper_osal.h"
static struct sunxi_osal_ops *os = &g_sunxi_os_ops;

#ifndef SUNXI_GPIOS_PER_BANK
#define SUNXI_GPIOS_PER_BANK	32
#endif
#define GPIOS_PER_BANK			SUNXI_GPIOS_PER_BANK

#if 1 /* print error info */
#define pin_assert(_cond, _ret, fmt, ...) \
	do { \
		if (!(_cond)) { \
			asrm_err(fmt, ##__VA_ARGS__); \
			return _ret; \
		} \
	} while (0)
#endif

int pin_name_check(const char *pin)
{
	pin_assert((os->strlen(pin) >= os->strlen("PA0")), -1, "pin str len error! %s\n", pin);
	pin_assert((pin[0] == 'P'), -1, "pin str error! %s\n", pin);
	pin_assert((pin[1] >= 'A' && pin[1] <= 'M'), -1, "pin str error! %s\n", pin);
	pin_assert((os->strtoul(&pin[2], NULL, 10) <= GPIOS_PER_BANK), -1, "pin num error! %s\n", pin);

	return 0;
}

unsigned int pin_name_to_gpio_id(const char *pin)
{
	return (pin[1] - 'A') * GPIOS_PER_BANK + os->strtoul(&pin[2], NULL, 10);
}

void gpio_id_to_pin_name(unsigned int gpio_id, char *pin)
{
	unsigned int num;
	pin[0] = 'P';
	pin[1] = 'A' + gpio_id / GPIOS_PER_BANK;
	num = gpio_id % GPIOS_PER_BANK;
	if (num >= 10) {
		pin[2] = '0' + num / 10;
		pin[3] = '0' + num % 10;
		pin[4] = '\0';
	} else {
		pin[2] = '0' + num;
		pin[3] = '\0';
	}
}
