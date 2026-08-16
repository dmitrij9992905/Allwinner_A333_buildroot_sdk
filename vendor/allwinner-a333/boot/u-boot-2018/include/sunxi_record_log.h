/*
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef _SUNXI_RECORD_LOG_H_
#define _SUNXI_RECORD_LOG_H_

struct record_log_info {
	unsigned int magic;
	unsigned int log_size;
	char log_buf[0];
};

void sunxi_record_log(char *str);

void sunxi_flush_log(void);

#endif /* _SUNXI_RECORD_LOG_H_ */
