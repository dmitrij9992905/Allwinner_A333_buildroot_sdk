# SPDX-License-Identifier: GPL-2.0-or-later

ifeq ($(CONFIG_COMPONENTS_AMP_USER_RESOURCE), y)
CFLAGS += -I $(BASE)/components/common/aw/amp_app/amp_user_resource/
endif

obj-y += asrm_core.o

obj-$(CONFIG_AW_ASRM_PROVIDE_PERI_CLK_INFO) += peri_clk.o

obj-y += helper/
