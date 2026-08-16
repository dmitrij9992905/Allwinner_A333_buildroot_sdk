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

#include "asrm_core.h"

#ifdef AMP_SYS_RSC_MANAGER_ON_LINUX_KERNEL

#include <linux/of.h>

#define MAX_CLK_CONTROLLER_NUM 8
#define MAX_CLK_NUM_PER_CLK_CONTROLLER 256

typedef struct peri_clk_info {
	struct device_node *cc_node;
	uint32_t clk_cnt;
	uint32_t clk_id[MAX_CLK_NUM_PER_CLK_CONTROLLER];
} peri_clk_info_t;

static uint32_t g_clk_controller_cnt;
static peri_clk_info_t *g_peri_clk_info[MAX_CLK_CONTROLLER_NUM];

static int g_is_clk_info_init;
static int g_is_dump_clk_info;

static const char *g_asrm_node_name = "amp_system_resource_manager";
static const char *g_peri_mgr_prop = "peripheral_resource_manager";

static const char *g_parse_peri_clk_prop = "parse_peri_clk";
static const char *g_dump_clk_prop = "dump_peri_clk_info";

static inline int is_clk_exist(const peri_clk_info_t *clk_info, uint32_t clk_id)
{
	uint32_t i;
	for (i = 0; i < clk_info->clk_cnt; i++) {
		if (clk_info->clk_id[i] == clk_id)
			return 1;
	}

	return 0;
}

static int add_clk_info(peri_clk_info_t *clk_info, uint32_t clk_id)
{
	if (clk_info->clk_cnt >= MAX_CLK_NUM_PER_CLK_CONTROLLER) {
		asrm_err("the clock num of '%s' is greater than %d, "
			"please increase MAX_CLK_NUM_PER_CLK_CONTROLLER!",
			clk_info->cc_node->full_name, MAX_CLK_NUM_PER_CLK_CONTROLLER);
		return -1;
	}

	clk_info->clk_id[clk_info->clk_cnt] = clk_id;
	clk_info->clk_cnt++;
	return 0;
}

static int add_peri_clk_info(struct device_node *node, uint32_t clk_id)
{
	int ret, is_clk_controller_found;
	uint32_t i;
	peri_clk_info_t *clk_info;

	is_clk_controller_found = 0;
	for (i = 0; i < g_clk_controller_cnt; i++) {
		clk_info = g_peri_clk_info[i];
		if (clk_info->cc_node == node) {
			is_clk_controller_found = 1;
			break;
		}
	}

	if (is_clk_controller_found) {
		if (!is_clk_exist(clk_info, clk_id)) {
			ret = add_clk_info(clk_info, clk_id);
			if (ret)
				return -1;
		}

		return 0;
	}

	if (g_clk_controller_cnt >= MAX_CLK_CONTROLLER_NUM) {
		asrm_err("can't add new clock controller '%s'", node->full_name);
		asrm_err("clock controller's num is greater than %d, "
			"please increase MAX_CLK_CONTROLLER_NUM!",
			MAX_CLK_CONTROLLER_NUM);
		return -2;
	}

	clk_info = asrm_port_malloc(sizeof(peri_clk_info_t));
	if (!clk_info) {
		asrm_err("mem allocation for peri clk info failed\n");
		return -3;
	}

	clk_info->cc_node = node;
	clk_info->clk_cnt = 0;
	ret = add_clk_info(clk_info, clk_id);
	if (ret) {
		asrm_port_free(clk_info);
		return -4;
	}

	g_peri_clk_info[g_clk_controller_cnt] = clk_info;
	g_clk_controller_cnt++;
	return 0;
}

static int find_target_node(struct device_node **node)
{
	struct device_node *asrm_node, *peri_mgr_node, *target_node;

	asrm_node = of_find_node_by_name(NULL, g_asrm_node_name);
	if (!asrm_node) {
		asrm_err("ASRM root node '%s' not exist", g_asrm_node_name);
		return -1;
	}

	peri_mgr_node = of_get_child_by_name(asrm_node, g_peri_mgr_prop);
	if (!peri_mgr_node) {
		asrm_err("peri manager node '%s' not exist", g_peri_mgr_prop);
		return -2;
	}

	target_node = of_find_node_with_property(peri_mgr_node, g_parse_peri_clk_prop);
	if (!target_node) {
		asrm_err("can't find dts node with property '%s'", g_parse_peri_clk_prop);
		return -3;
	}

	asrm_dbg("target node full name: '%s'", target_node->full_name);
	*node = target_node;
	return 0;
}

static int parse_peri_clk(struct device_node *peri_node)
{
	int ret, i, elem_cnt, clk_num, parse_success_cnt;
	const char *property_name;
	struct of_phandle_args clk_args;

	asrm_dbg("peri full name: '%s'", peri_node->full_name);

	property_name = "clocks";
	elem_cnt = of_property_count_u32_elems(peri_node, property_name);
	if (elem_cnt <= 0) {
		asrm_err("node '%s' with invalid property '%s', ret: %d",
			peri_node->full_name, property_name, elem_cnt);
		return -1;
	}

	if (elem_cnt % 2) {
		asrm_warn("the '%s' property of node(%s) has %d element(32bit)!",
			property_name, peri_node->full_name, elem_cnt);
	}

	clk_num = (elem_cnt + 1)/ 2;
	asrm_dbg("peri clk num: %d", clk_num);

	parse_success_cnt = 0;
	for (i = 0; i < clk_num; i++) {
		ret = of_parse_phandle_with_args(peri_node, property_name, "#clock-cells", i, &clk_args);
		if (ret) {
			asrm_err("parse '%s' phandle failed, index: %d, ret: %d", property_name, i, ret);
			continue;
		}

		if (g_is_dump_clk_info)
			of_print_phandle_args("peri clk args:", &clk_args);

		if (clk_args.args_count != 1) {
			asrm_err("unsupported clk controller phandle args, node: '%s', clk_index: %d",
			peri_node->full_name, i);
			of_print_phandle_args("unsupported clk args:", &clk_args);
			continue;
		}

		ret = add_peri_clk_info(clk_args.np, clk_args.args[0]);
		if (ret) {
			asrm_err("add_peri_clk_info failed, node: '%s', clk_index: %d",
			peri_node->full_name, i);
			of_print_phandle_args("current clk args:", &clk_args);
			continue;
		}
		parse_success_cnt++;
	}

	if (!parse_success_cnt)
		return -2;

	return 0;
}

static int asrm_generate_peri_clk_info(void)
{
	int ret, i, peri_cnt, parse_success_cnt;
	const char *property_name;
	struct device_node *target_node, *peri_node;

	ret = find_target_node(&target_node);
	if (ret) {
		return -1;
	}

	g_is_dump_clk_info = of_property_read_bool(target_node, g_dump_clk_prop);

	property_name = "devices";
	peri_cnt = of_property_count_u32_elems(target_node, property_name);
	if (peri_cnt <= 0) {
		asrm_err("node '%s' with invalid property '%s', ret: %d",
			target_node->full_name, property_name, peri_cnt);
		return -2;
	}

	parse_success_cnt = 0;
	for (i = 0; i < peri_cnt; i++) {
		peri_node = of_parse_phandle(target_node, property_name, i);
		if (!peri_node) {
			asrm_err("peri node phandle parse failed, phandle index: %d", i);
			continue;
		}

		ret = parse_peri_clk(peri_node);
		if (ret) {
			asrm_err("parse peri '%s' clk failed, ret: %d", peri_node->full_name, ret);
			continue;
		}
		parse_success_cnt++;
	}

	if (!parse_success_cnt)
		return -3;

	asrm_info("generate peripheral clk info success!");
	return 0;
}

static int dump_peri_clk_info(void)
{
	uint32_t i, j;
	peri_clk_info_t *clk_info;

	asrm_info("clk controller cnt: %u", g_clk_controller_cnt);

	for (i = 0; i < g_clk_controller_cnt; i++) {
		clk_info = g_peri_clk_info[i];
		if (!clk_info) {
			asrm_warn("clk info is null, index: %u", i);
			continue;
		}

		asrm_info("clk controller: '%s', clk_cnt: %u", clk_info->cc_node->full_name, clk_info->clk_cnt);
		for (j = 0; j < clk_info->clk_cnt; j++) {
			asrm_info("clk_id[%u]=%u", j, clk_info->clk_id[j]);
		}
	}
	return 0;
}

int sunxi_of_is_rproc_peri_clk(struct device_node *node, uint32_t clk_id)
{
	int ret;
	uint32_t i, j;
	peri_clk_info_t *clk_info;

	if (!g_is_clk_info_init) {
		ret = asrm_generate_peri_clk_info();
		if (ret) {
			g_is_clk_info_init = -1;
			asrm_err("asrm_generate_peri_clk_info failed, ret: %d", ret);
		} else {
			g_is_clk_info_init = 1;
			if (g_is_dump_clk_info)
				dump_peri_clk_info();
		}
	}

	if (g_is_clk_info_init < 0) {
		return 0;
	}

	for (i = 0; i < g_clk_controller_cnt; i++) {
		clk_info = g_peri_clk_info[i];
		if (!clk_info)
			continue;

		if (clk_info->cc_node != node) {
			continue;
		}

		for (j = 0; j < clk_info->clk_cnt; j++) {
			if (clk_info->clk_id[j] == clk_id)
				return 1;
		}
	}
	return 0;
}

#endif
