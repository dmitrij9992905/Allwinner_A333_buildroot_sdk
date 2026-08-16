/*
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <sunxi_record_log.h>
#include <sys_partition.h>
#include <memalign.h>

#define RECORD_LOG_STATIC_BUF_SIZE		2048
#define RECORD_LOG_DYNAMIC_BUF_SIZE		32768
#define BOOT_LOG_PARTITION_MAGIC		0x66768077 /*B L P M*/
#define FLASH_BLK_SIZE					512
#define	ONLY_ONE_BLK					1

__attribute__((section(".data"))) static char sunxi_record_log_static_buf[RECORD_LOG_STATIC_BUF_SIZE];
__attribute__((section(".data"))) static bool is_flash_init;

static char *sunxi_record_log_dynamic_buf;

static int log_raw_write(int log_size, char *write_log)
{
	int ret_val;
	char *write_buf;
	char *log_buf;
	struct record_log_info *write_record_info;
	int before_last_block;
	int before_log_offset;

	static bool is_partition_init;
	static unsigned int partition_start;
	static unsigned int partition_size;
	static struct record_log_info *before_record_info;

	if (!is_partition_init) {
		ret_val = sunxi_partition_get_info_byname(CONFIG_SUNXI_RECORD_LOG_PARTITION, &partition_start, &partition_size);
		if (ret_val) {
			puts("get boot_log patition info is error\n");
			return -1;
		}
		before_record_info = memalign(CACHE_LINE_SIZE, FLASH_BLK_SIZE);
		if (!before_record_info) {
			puts("log raw write malloc head buf is error\n");
			return -1;
		}
		sunxi_flash_read(partition_start, 1, before_record_info);
		if (before_record_info->magic == BOOT_LOG_PARTITION_MAGIC)
			is_partition_init = 1;
	}

	if (is_partition_init) {
		if ((before_record_info->log_size + log_size) > (partition_size * FLASH_BLK_SIZE)) {
			puts("boot log size is larger than partition size\n");
			return -1;
		}
		before_last_block = ALIGN((before_record_info->log_size + sizeof(struct record_log_info)), FLASH_BLK_SIZE) / FLASH_BLK_SIZE;
		before_log_offset = (before_record_info->log_size + sizeof(struct record_log_info)) % FLASH_BLK_SIZE;
		before_record_info->log_size += log_size;

		write_buf = memalign(CACHE_LINE_SIZE, before_log_offset + log_size);
		if (!write_buf) {
			puts("malloc write buf is error\n");
			return -1;
		}

		if (before_log_offset) {
			log_buf = memalign(CACHE_LINE_SIZE, FLASH_BLK_SIZE);
			if (!log_buf) {
				puts("log read buf malloc is error\n");
				return -1;
			}

			sunxi_flash_read(partition_start + before_last_block - 1, 1, log_buf);
			memcpy(write_buf, log_buf, before_log_offset);
			memcpy(write_buf + before_log_offset, write_log, log_size);
			sunxi_flash_write(partition_start + before_last_block - 1, \
				ALIGN((before_log_offset + log_size), FLASH_BLK_SIZE) / FLASH_BLK_SIZE, \
				(void *)write_buf);

			if (before_last_block == ONLY_ONE_BLK) {
				write_record_info = (struct record_log_info *)write_buf;
				write_record_info->log_size = before_record_info->log_size;
				memcpy(before_record_info, write_buf, FLASH_BLK_SIZE);
			}
			free(log_buf);
		} else {
			memcpy(write_buf, write_log, log_size);
			sunxi_flash_write(partition_start + before_last_block, \
				ALIGN(log_size, FLASH_BLK_SIZE) / FLASH_BLK_SIZE, \
				(void *)write_buf);
		}
		free(write_buf);
		sunxi_flash_write(partition_start, 1, (void *)before_record_info);

	} else {
		write_record_info = memalign(CACHE_LINE_SIZE, log_size + sizeof(struct record_log_info));
		if (!write_record_info) {
			puts("malloc write_record_info is error\n");
			return -1;
		}
		write_record_info->magic = BOOT_LOG_PARTITION_MAGIC;
		write_record_info->log_size = log_size;
		memcpy(write_record_info->log_buf, write_log, log_size);
		sunxi_flash_write(partition_start, \
			ALIGN((log_size + sizeof(struct record_log_info)), FLASH_BLK_SIZE) / FLASH_BLK_SIZE, \
			(void *)write_record_info);
		memcpy(before_record_info, write_record_info, FLASH_BLK_SIZE);
		free(write_record_info);
		is_partition_init = 1;
	}

	return 0;
}

static void record_log(struct record_log_info *record_info, char *str, int buf_size)
{
	int record_log_size, str_size;
	char *record_log_buf;

	record_log_size = record_info->log_size;
	record_log_buf  = record_info->log_buf;
	str_size        = strlen(str);

	if (buf_size > record_log_size + str_size + sizeof(struct record_log_info)) {
		sprintf(record_log_buf + record_log_size, "%s", str);
		record_info->log_size = record_log_size + str_size + 1;
	} else {
		puts("record log size is larger than buf size\n");
	}

	return;
}

static void init_dynamic_record_log_buf(void)
{
	sunxi_record_log_dynamic_buf = malloc(RECORD_LOG_DYNAMIC_BUF_SIZE);
	if (sunxi_record_log_dynamic_buf)
		memcpy(sunxi_record_log_dynamic_buf, sunxi_record_log_static_buf, RECORD_LOG_STATIC_BUF_SIZE);
	else
		puts("malloc dynamic record log buf is error\n");

	return;
}

static void record_log_to_buf(char *str)
{
	static bool dynamic_buf_init;
	static struct record_log_info *record_info = (struct record_log_info *)sunxi_record_log_static_buf;
	static int record_buf_size = RECORD_LOG_STATIC_BUF_SIZE;

	if (gd->flags & GD_FLG_RELOC) {
		if (!dynamic_buf_init) {
			init_dynamic_record_log_buf();
			record_info = (struct record_log_info *)sunxi_record_log_dynamic_buf;
			record_buf_size = RECORD_LOG_DYNAMIC_BUF_SIZE;
			dynamic_buf_init = 1;
		}

		if (sunxi_record_log_dynamic_buf)
			record_log(record_info, str, record_buf_size);
	} else {
		record_log(record_info, str, record_buf_size);
	}

	return;
}

static void record_log_to_flash(char *str)
{
	log_raw_write(strlen(str) + 1, str);
	return;
}

void sunxi_record_log(char *str)
{
	if (is_flash_init)
		record_log_to_flash(str);
	else
		record_log_to_buf(str);

	return;
}

void sunxi_flush_log(void)
{
	struct record_log_info *now_record_info;
	int ret_val;

	now_record_info = (struct record_log_info *)sunxi_record_log_dynamic_buf;
	if (now_record_info) {
		ret_val = log_raw_write(now_record_info->log_size, now_record_info->log_buf);
		if (!ret_val) {
			free(sunxi_record_log_dynamic_buf);
			sunxi_record_log_dynamic_buf = NULL;
			is_flash_init = 1;
		}
	}

	return;
}
