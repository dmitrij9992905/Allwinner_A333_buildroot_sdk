// SPDX-License-Identifier: GPL-2.0-only

#ifndef _BOOTTIME_RECORD_H_
#define _BOOTTIME_RECORD_H_
#define MAX_RECORD_BUF_LEN	112

struct sunxi_module_probe_time_record_t {
	int pid;
	ktime_t start_time;
	char drv_dev_name[MAX_RECORD_BUF_LEN];
	struct list_head list_node;
};

#endif /* _BOOTTIME_RECORD_H_ */