// SPDX-License-Identifier: GPL-2.0-only

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/clockchips.h>
#include <linux/sched/clock.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/device/driver.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <sunxi-log.h>
#include <linux/init.h>
#include "boottime_recorder.h"

#define MAX_SYMBOL_LEN	64
#define MSG_SIZE		128
#define MODULE_PROBE_TIME_THRESH 10 /* ms */
#define SUNXI_RECORD_BOOT_TIME_VERSION	"0.0.1"

static int probe_time_thresh = MODULE_PROBE_TIME_THRESH;

static char module_probe_kprobe_function[MAX_SYMBOL_LEN] = "really_probe";

static struct kprobe module_probe_time_record = {
	.symbol_name	= module_probe_kprobe_function,
};

static struct kretprobe module_probe_time_record_kretprobe = {
	/* Probe up to 20 instances concurrently. */
	.maxactive		= 20,
};

extern void log_boot(char *str);

static LIST_HEAD(module_probe_time_record_list);
static DEFINE_MUTEX(module_probe_time_record_mutex);

static int __kprobes module_probe_time_record_handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	struct device *dev;
	struct device_driver *drv;
	struct sunxi_module_probe_time_record_t *probe_time_record;

	dev = (struct device *)regs->regs[0];
	drv = (struct device_driver *)regs->regs[1];

	probe_time_record = kzalloc(sizeof(struct sunxi_module_probe_time_record_t), \
								GFP_ATOMIC | __GFP_NORETRY | __GFP_NOWARN);

	if (probe_time_record) {
		snprintf(probe_time_record->drv_dev_name, MAX_RECORD_BUF_LEN, "%s: %s",
			     (drv->name ? drv->name : "NULL"),\
			     (dev_name(dev) ? dev_name(dev) : "NULL"));

		probe_time_record->pid = current->pid;

		probe_time_record->start_time = local_clock();

		INIT_LIST_HEAD(&probe_time_record->list_node);

		mutex_lock(&module_probe_time_record_mutex);

		list_add_tail(&probe_time_record->list_node, &module_probe_time_record_list);

		mutex_unlock(&module_probe_time_record_mutex);
	}

	return 0;
}

static int module_probe_time_record_handler_ret(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	static ktime_t aw_module_probe_end_time;
	char msgbuf[MSG_SIZE];
	u64 aw_module_probe_use_time;
	struct sunxi_module_probe_time_record_t *pos, *temp;

	mutex_lock(&module_probe_time_record_mutex);

	list_for_each_entry_safe(pos, temp, &module_probe_time_record_list, list_node) {
		if (pos->pid == current->pid) {
			aw_module_probe_end_time = local_clock();
			aw_module_probe_use_time = (unsigned long long)ktime_ms_delta(aw_module_probe_end_time, pos->start_time);
			if (aw_module_probe_use_time >= probe_time_thresh) {
				snprintf(msgbuf, MSG_SIZE, "%s: %lld ms", pos->drv_dev_name, aw_module_probe_use_time);
				log_boot(msgbuf);
			}
			list_del(&pos->list_node);
			kfree(pos);
		}
	}

	mutex_unlock(&module_probe_time_record_mutex);

	return 0;
}

#ifndef MODULE
static int __init get_probe_time_thresh(char *str)
{
	if (str && strlen(str))
		probe_time_thresh = simple_strtol(str, NULL, 10);

	return 0;
}

__setup("probe_time_thresh=", get_probe_time_thresh);
#endif /* MODULE */

static int __init module_probe_time_record_init(void)
{
	int ret;

	module_probe_time_record.pre_handler = module_probe_time_record_handler_pre;
	ret = register_kprobe(&module_probe_time_record);
	if (ret) {
		sunxi_err(NULL, "module_probe_time_record kprobe register error\n");
		return ret;
	}

	module_probe_time_record_kretprobe.kp.symbol_name = module_probe_kprobe_function;
	module_probe_time_record_kretprobe.handler = module_probe_time_record_handler_ret;
	ret = register_kretprobe(&module_probe_time_record_kretprobe);
	if (ret) {
		sunxi_err(NULL, "module_probe_time_record_kretprobe kretprobe register error\n");
		unregister_kprobe(&module_probe_time_record);
		return ret;
	}

	return 0;
}

static int __init sunxi_btr_init(void) /* btr: boot time record */
{
	int ret;
	ret = module_probe_time_record_init();
	if (ret)
		return ret;

	return 0;
}

static void __exit module_probe_time_record_exit(void)
{
	unregister_kprobe(&module_probe_time_record);
	unregister_kretprobe(&module_probe_time_record_kretprobe);
}

static void __exit sunxi_btr_exit(void) /* btr: boot time record */
{
	module_probe_time_record_exit();
}

early_initcall(sunxi_btr_init)
module_exit(sunxi_btr_exit)

MODULE_AUTHOR("weizhouxiang <weizhouxiang@allwinnertech.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(SUNXI_RECORD_BOOT_TIME_VERSION);
