// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024-2026
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * maguojun <maguojun@allwinnertech.com>
 *
 */
#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/arch/rtc.h>

#define CRC_FLAG        (0x1b1b1b1b)
#define RTC_CRC_INDEX   3

int do_rtc_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long ret, original_value;

	original_value = rtc_read_data(RTC_CRC_INDEX);
	rtc_write_data(RTC_CRC_INDEX, CRC_FLAG);

	ret = rtc_read_data(RTC_CRC_INDEX);
	if (ret != (unsigned long)CRC_FLAG) {
		printf("RTC gpr test failed\n");
	} else {
		printf("RTC gpr test successed\n");
	}

	rtc_write_data(RTC_CRC_INDEX, original_value);

	return 0;
}

U_BOOT_CMD(
	rtc_test, 1, 0, do_rtc_test,
	"do rtc test",
	"sunxi rtc"
);
