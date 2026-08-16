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

#include <stdio.h>
#include <string.h>

#include <sunxi_amp_rsc.h>
#include "asrm_test.h"


#define asrm_test_log_without_newline(fmt, ...) \
			printf(fmt, ##__VA_ARGS__)

#define asrm_test_log(fmt, ...) asrm_test_log_without_newline(fmt"\n", ##__VA_ARGS__)

typedef struct asrm_test_case {
	const char *desc;
	sunxi_amp_rsc_req_info_t rsc_req_info;
	sunxi_amp_rsc_t rsc;
	int is_req_api_ret_zero;
	int is_free_api_ret_zero;
	int is_permission_api_ret_zero;
} asrm_test_case_t;

typedef struct asrm_test {
	uint32_t test_case_cnt;
	asrm_test_case_t test_case[9];
	//sunxi_amp_rsc_t rsc[];
} asrm_test_t;

asrm_test_t g_asrm_test;

static void dump_peri_desc(const sunxi_peri_rsc_desc_t *desc)
{
	printf("start: 0x%08lx\n", desc->start_addr);
	printf("len: 0x%08x\n", desc->len);
}

static void dump_asrm_test_case(const asrm_test_case_t *tc)
{
	const sunxi_amp_rsc_req_info_t *rsc_req_info;
	sunxi_amp_rsc_type_t rsc_type;

	rsc_req_info = &tc->rsc_req_info;

	rsc_type = rsc_req_info->rsc_type;
	printf("Description: '%s'\n", tc->desc);
	printf("RSC type: %d\n", rsc_type);
	printf("SW module: '%s'\n", rsc_req_info->sw_module_id_str);
	if (rsc_type == SUNXI_AMP_RSC_HW_PERI) {
		printf("Peripheral resource\n");
		dump_peri_desc(&rsc_req_info->peri);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_GPIO) {
		printf("GPIO resource\n");
		dump_peri_desc(&rsc_req_info->gpio.peri);
		printf("GPIO ID: %u\n", rsc_req_info->gpio.gpio_id);
	} else if (rsc_type == SUNXI_AMP_RSC_HW_DMA_CHANNEL) {
		printf("DMA channel resource\n");
		dump_peri_desc(&rsc_req_info->dma.peri);
		printf("Channel ID: %u\n", rsc_req_info->dma.channel_id);
	} else {
		printf("Unknown resource type\n");
	}

	printf("req api return zero: %d\n", tc->is_req_api_ret_zero);
	printf("free api return zero: %d\n", tc->is_free_api_ret_zero);
	printf("permission api return zero: %d\n\n", tc->is_permission_api_ret_zero);
}

static int req_api_test(asrm_test_case_t *tc)
{
	int ret;

	ret = sunxi_amp_rsc_request(&tc->rsc_req_info, &tc->rsc);
	if (tc->is_req_api_ret_zero) {
		if (ret) {
			asrm_test_log("amp rsc request failed, ret shoule be zero, but actual is %d", ret);
			return -1;
		}
	} else {
		if (!ret) {
			asrm_test_log("amp rsc request failed, ret shoule be not zero, but actual is %d", ret);
			return -1;
		}
	}

	return 0;
}

static __attribute__((__unused__)) int free_api_test(asrm_test_case_t *tc)
{
	int ret;

	ret = sunxi_amp_rsc_free(tc->rsc);
	if (tc->is_free_api_ret_zero) {
		if (ret) {
			asrm_test_log("amp rsc free failed, ret shoule be zero, but actual is %d", ret);
			return -1;
		}
	} else {
		if (!ret) {
			asrm_test_log("amp rsc free failed, ret shoule be not zero, but actual is %d", ret);
			return -1;
		}
	}

	return 0;

}

static int permission_api_test(asrm_test_case_t *tc)
{
	int ret;

	ret = sunxi_amp_rsc_has_permission(tc->rsc);
	if (tc->is_permission_api_ret_zero) {
		if (ret) {
			asrm_test_log("amp rsc permission verify failed, ret shoule be zero, but actual is %d", ret);
			return -1;
		}
	} else {
		if (!ret) {
			asrm_test_log("amp rsc permission verify failed, ret shoule be not zero, but actual is %d", ret);
			return -1;
		}
	}

	return 0;

}

static __attribute__((__unused__)) int asrm_test_case_init(asrm_test_case_t *tc, const char *desc, sunxi_amp_rsc_req_info_t *rsc_req_info)
{
	return 0;
}

static int asrm_test_init(asrm_test_t *test)
{
	int index;
	asrm_test_case_t *tc;
	sunxi_amp_rsc_req_info_t *rsc_req_info;

	test->test_case_cnt = 9;

	index = 0;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "has_permission_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_PERI;
	rsc_req_info->peri.start_addr = 0x04026000;//SPI1
	rsc_req_info->peri.len = 0x1000;
	tc->is_req_api_ret_zero = 1;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 0;

	index = 1;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "no_permission_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_PERI;
	rsc_req_info->peri.start_addr = 0x04025000;//SPI0
	rsc_req_info->peri.len = 0x1000;
	tc->is_req_api_ret_zero = 0;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 1;

	index = 2;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "implicit_alloc_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_PERI;
	rsc_req_info->peri.start_addr = 0x04020000;
	rsc_req_info->peri.len = 0x1000;
	tc->is_req_api_ret_zero = 1;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 0;


	index = 3;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "has_permission_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_GPIO;
	rsc_req_info->gpio.peri.start_addr = 0x03604000;
	rsc_req_info->gpio.peri.len = 0x900;
	rsc_req_info->gpio.gpio_id = 0x162;
	tc->is_req_api_ret_zero = 1;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 0;

	index = 4;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "no_permission_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_GPIO;
	rsc_req_info->gpio.peri.start_addr = 0x03604000;
	rsc_req_info->gpio.peri.len = 0x900;
	rsc_req_info->gpio.gpio_id = 0;
	tc->is_req_api_ret_zero = 0;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 1;

	index = 5;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "implicit_alloc_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_GPIO;
	rsc_req_info->gpio.peri.start_addr = 0x03604000;
	rsc_req_info->gpio.peri.len = 0x900;
	rsc_req_info->gpio.gpio_id = 1;
	tc->is_req_api_ret_zero = 1;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 0;


	index = 6;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "has_permission_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_DMA_CHANNEL;
	rsc_req_info->dma.peri.start_addr = 0x4000000;//0x3001000;
	rsc_req_info->dma.peri.len = 0x1000;
	rsc_req_info->dma.channel_id = 4;
	tc->is_req_api_ret_zero = 1;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 0;

	index = 7;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "no_permission_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_DMA_CHANNEL;
	rsc_req_info->dma.peri.start_addr = 0x4000000;//0x4000000
	rsc_req_info->dma.peri.len = 0x1000;
	rsc_req_info->dma.channel_id = 1;
	tc->is_req_api_ret_zero = 0;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 1;

	index = 8;
	tc = &test->test_case[index];
	memset(tc, 0, sizeof(*tc));
	tc->desc = "implicit_alloc_test";
	rsc_req_info = &tc->rsc_req_info;
	rsc_req_info->sw_module_id_str = "ASRM_TEST";
	rsc_req_info->rsc_type = SUNXI_AMP_RSC_HW_DMA_CHANNEL;
	rsc_req_info->dma.peri.start_addr = 0x4000000;
	rsc_req_info->dma.peri.len = 0x1000;
	rsc_req_info->dma.channel_id = 0;
	tc->is_req_api_ret_zero = 1;
	tc->is_free_api_ret_zero = 1;
	tc->is_permission_api_ret_zero = 0;

	return 0;
}

int asrm_test(void)
{
	int ret;
	uint32_t i;

	asrm_test_t *test;
	asrm_test_case_t *tc;

	test = &g_asrm_test;
	ret = asrm_test_init(test);
	if (ret)
		return -1;

	for (i = 0; i < test->test_case_cnt; i++) {
		tc = &test->test_case[i];
		asrm_test_log("ASRM test case %u:", i);
		dump_asrm_test_case(tc);
		ret = req_api_test(tc);
		if (ret) {
			asrm_test_log("req_api_test failed, tc index: %u", i);
			return -1;
		}

		asrm_test_log("Resource handle: %p", tc->rsc);
		ret = permission_api_test(tc);
		if (ret) {
			asrm_test_log("permission_api_test failed, tc index: %u", i);
			return -1;
		}

		ret = free_api_test(tc);
		if (ret) {
			asrm_test_log("free_api_test failed, tc index: %u", i);
			return -1;
		}

		asrm_test_log("ASRM test case %u pass!\n", i);
	}

	return 0;
}
#include <hal_cmd.h>

static int cmd_asrm_test(int argc, char *argv[])
{
	asrm_test();
	return 0;
}
FINSH_FUNCTION_EXPORT_CMD(cmd_asrm_test, asrm_test, asrm test);

