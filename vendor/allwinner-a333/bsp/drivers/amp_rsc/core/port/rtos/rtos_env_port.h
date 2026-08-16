/* SPDX-License-Identifier: GPL-2.0-only */
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


#ifndef __RTOS_ENV_PORT_H__
#define __RTOS_ENV_PORT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aw_list.h>
#include <hal_mutex.h>


#define asrm_printf printf

#define ASRM_LOG_COLOR_NONE "\e[0m"
#define ASRM_LOG_COLOR_RED "\e[31m"
#define ASRM_LOG_COLOR_GREEN "\e[32m"
#define ASRM_LOG_COLOR_YELLOW "\e[33m"
#define ASRM_LOG_COLOR_BLUE "\e[34m"

typedef struct hal_mutex asrm_port_mutex_t;

static inline int asrm_port_mutex_lock(asrm_port_mutex_t *mutex)
{
	return hal_mutex_lock(mutex);
}

static inline int asrm_port_mutex_unlock(asrm_port_mutex_t *mutex)
{
	return hal_mutex_unlock(mutex);
}

static inline void *asrm_port_malloc(size_t size)
{
	return malloc(size);
}

static inline void *asrm_port_zalloc(size_t size)
{
	void *ptr;
	ptr = asrm_port_malloc(size);
	if (ptr)
		memset(ptr, 0, size);

	return ptr;
}

static inline void asrm_port_free(void *ptr)
{
	free(ptr);
}

typedef unsigned int gfp_t;
static inline void *asrm_port_kmalloc(size_t size, gfp_t flags)
{
	return malloc(size);
}

static inline void *asrm_port_kzalloc(size_t size, gfp_t flags)
{
	return asrm_port_zalloc(size);
}

static inline void asrm_port_kfree(const void *mem)
{
	free((void *)mem);
}

#endif /* __RTOS_ENV_PORT_H__ */
