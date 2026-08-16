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

#include <linux/kernel.h>
#include <linux/slab.h>
#include "helper_osal.h"

static unsigned long ___strlen(const char *str)
{
	return strlen(str);
}

static unsigned long ___strtoul(const char *cp, char **endp, unsigned int base)
{
	return simple_strtoul(cp, endp, base);
}

static void *___zalloc(unsigned int size)
{
	return kzalloc(size, GFP_KERNEL);
}

static void ___free(void *ptr)
{
	kfree(ptr);
}

static char *___strncpy(char *dest, const char *src, unsigned long n)
{
	return strncpy(dest, src, n);
}

static void *___memcpy(void *dst, const void *src, unsigned long n)
{
	return memcpy(dst, src, n);
}

static void *___memset(void *str, int c, unsigned long n)
{
	return memset(str, c, n);
}

static int ___print(const char *fmt, ...)
{
	va_list args;
	uint i;

	va_start(args, fmt);
	i = vprintk(fmt, args);
	va_end(args);

	return i;
}

struct sunxi_osal_ops g_sunxi_os_ops = {
	.zalloc  = ___zalloc,
	.free    = ___free,
	.strlen  = ___strlen,
	.strtoul = ___strtoul,
	.strncpy = ___strncpy,
	.memcpy  = ___memcpy,
	.memset  = ___memset,
	.print   = ___print,
};
