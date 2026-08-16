/*
 * (C) Copyright 2017-2018
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * Configuration settings for the Allwinner sunxi series of boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <efuse_map.h>
#include <asm/sbi.h>
#include <sbi.h>
#include <securestorage.h>
#include <asm/arch/ce.h>

DECLARE_GLOBAL_DATA_PTR;

#define SBI_EXT_SUNXI			0x54535251

/*efuse*/
#define SBI_EXT_SUNXI_EFUSE_WRITE	0
#define SBI_EXT_SUNXI_EFUSE_READ	1

/*hdcp*/
#define SBI_EXT_SUNXI_DEAL_HDCP_KEY		0x10
#define SBI_EXT_SUNXI_INSTALL_HDCP_KEY	0x11

#define SBI_EXT_SUNXI_BOX_STANDBY		0x20

#define SBI_EXT_TIME			0x54494D45
#define SBI_EXT_TIME_SET_TIMER	0

int sbi_ecall_efuse_write(void *key_buf)
{
	struct sbiret ret;
	efuse_key_info_t *keyinfo = (efuse_key_info_t *)key_buf;

	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_EFUSE_WRITE,
				(unsigned long)keyinfo, 0, 0, 0, 0, 0);
	if (ret.error) {
		pr_err("sbi error is %lu, value is %lu\n", ret.error, ret.value);
		return ret.error;
	}
	return ret.value;
}

int sbi_ecall_efuse_read(void *key_buf, void *read_buf, void *read_len)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_EFUSE_READ,
				(unsigned long)key_buf, (unsigned long)read_buf, (unsigned long)read_len, 0, 0, 0);
	if (ret.error) {
		pr_err("sbi error is %lu, value is %lu\n", ret.error, ret.value);
		return ret.error;
	}
	return ret.value;
}

void sbi_set_arch_timer(uint64_t stime_value)
{
	sbi_ecall(SBI_EXT_TIME, SBI_EXT_TIME_SET_TIMER, stime_value,
		stime_value >> 32, 0, 0, 0, 0);
}

#if IS_ENABLED(CONFIG_SUNXI_HDCP_NO_KEYBOX)
struct sbi_call_ctx {
	u64 intput_buf;
	u32 intput_len;
	u64 output_buf;
	u32 output_len;
};

typedef struct _aes_function_info {
	u32 decrypt;
	u64 key_addr;
	u32 key_len;
	u64 iv_map;
	u64 key_mode;
} aes_function_info;

int sbi_ecall_deal_hdcp_key(void *out_buf, int out_len, void *data_buf,
							int data_len, int hdcpkey_type, int aes_op)
{
	struct sbiret ret;
	aes_function_info aes_info;
	struct sbi_call_ctx call_ctx;

	if ((!out_buf) || (!data_buf)) {
		pr_err("input param is NULL\n ");
		return -1;
	}

	if (data_len > SUNXI_SECURE_STORTAGE_BLOCK_SIZE) {
		pr_err("data len is max than buf size\n ");
		return -2;
	}

	memset(&aes_info, 0x0, sizeof(aes_function_info));
	aes_info.decrypt = aes_op;//AES_ENCRYPT;
	aes_info.key_len = 128 / 8; /* key len is 128 bit */
	aes_info.key_mode = AES_MODE_CBC;

	memset(&call_ctx, 0x0, sizeof(struct sbi_call_ctx));
	call_ctx.intput_buf = (unsigned long)data_buf;
	call_ctx.intput_len = data_len;
	call_ctx.output_buf = (unsigned long)out_buf;
	call_ctx.output_len = out_len;

	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_DEAL_HDCP_KEY,
					(unsigned long)&call_ctx, hdcpkey_type,
					(unsigned long)&aes_info, 0, 0, 0);
	if (ret.error) {
		pr_err("sbi error is %lu, value is %lu\n", ret.error, ret.value);
		return -3;
	}

	return 0;
}

int sbi_ecall_install_hdcp_key(void *data_buf, int data_len, int hdcpkey_type)
{
	struct sbiret ret;

	if ((!data_len) || (!data_buf)) {
		pr_err("input param is NULL\n ");
		return -1;
	}

	if (data_len > SUNXI_SECURE_STORTAGE_BLOCK_SIZE) {
		pr_err("data len is max than buf size\n ");
		return -2;
	}

	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_INSTALL_HDCP_KEY, hdcpkey_type,
					(unsigned long)data_buf, data_len, 0, 0, 0);
	if (ret.error) {
		pr_err("sbi error is %lu, value is %lu\n", ret.error, ret.value);
		return -3;
	}

	return 0;
}
#endif

int sbi_ecall_box_standby(void)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_SUNXI, SBI_EXT_SUNXI_BOX_STANDBY,
					0, 0, 0, 0, 0, 0);
	if (ret.error) {
		pr_err("sbi error is %lu, value is %lu\n", ret.error, ret.value);
		return -3;
	}

	return 0;
}
