/******************************************************************************
 *
 * Copyright(c) 2020-2030  Seekwave Corporation.
 *
 *****************************************************************************/
#ifndef __BOOT_CONFIG_H__
#define __BOOT_CONFIG_H__
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include "skw_boot.h"
#define MODEM_ENABLE_GPIO -1
#define HOST_WAKEUP_GPIO_IN -1
#define MODEM_WAKEUP_GPIO_OUT -1
#define SEEKWAVE_NV_NAME "SEEKWAVE_NV_SWT6621S.bin"
//#define CONFIG_SEEKWAVE_FIRMWARE_LOAD
#define SKW_IRAM_FILE_PATH "/data/ROM_EXEC_KERNEL_IRAM.bin"
#define SKW_DRAM_FILE_PATH "/data/RAM_RW_KERNEL_DRAM.bin"
#define SKW_POWER_OFF_VALUE 0
#endif /* __BOOT_CONFIG_H__ */
