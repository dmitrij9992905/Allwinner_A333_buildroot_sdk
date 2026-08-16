/*
 * (C) Copyright 2018 allwinnertech  <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SBI_H__
#define __SBI_H__

#include <asm/io.h>

int sbi_ecall_efuse_write(void *key_buf);
int sbi_ecall_efuse_read(void *key_buf, void *read_buf, void *read_len);
void sbi_set_arch_timer(uint64_t stime_value);
int sbi_ecall_box_standby(void);

#if IS_ENABLED(CONFIG_SUNXI_HDCP_NO_KEYBOX)
int sbi_ecall_deal_hdcp_key(void *out_buf, int out_len, void *data_buf,
							int data_len, int hdcpkey_type, int aes_op);
int sbi_ecall_install_hdcp_key(void *data_buf, int data_len, int hdcpkey_type);
#endif

#endif
