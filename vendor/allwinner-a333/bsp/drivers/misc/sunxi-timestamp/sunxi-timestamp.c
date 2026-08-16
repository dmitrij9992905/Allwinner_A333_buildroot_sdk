// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/of.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/of_device.h>
#include <sunxi-log.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <sunxi-rtc.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/io.h>
#include "sunxi-timestamp.h"

#define DRV_NAME	"sunxi_timestamp"
#define DRV_VERSION "0.0.1"

static u64 timestamp_get(struct sunxi_timestamp_dev *chip)
{
	u64 timestamp;
	u32 t_low1;
	u32 t_low2;
	u32 t_high;
	long long t;

/* eg:
 * 1.t_low1 = 0xffffffff
 * 2.t_high = 0x00000001
 * After reading the low address and before reading the high address,
 * if the low address count is full, the system performs a tick count.
 * Data concatenation will cause offside, so you need to read the low
 * address status again to determine
 */
	do {
		t_low1 = readl((chip->base));
		t_high = readl((chip->base + 0x0004));
		t_low2 = readl((chip->base));
	} while (t_low2 < t_low1);
	t = ((long long)t_high << 32) | t_low1;
	timestamp = t / (chip->hosc_fre / 1000);

	return timestamp;
}

static int timestamp_proc_show(struct seq_file *m, void *v)
{
	u64 timestamp;
	struct sunxi_timestamp_dev *chip = m->private;

	timestamp = timestamp_get(chip);

	seq_printf(m, "%lld\n", timestamp);
	return 0;
}

static int sunxi_timestamp_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct clk *hosc_clock;
	struct sunxi_timestamp_dev *chip;

	chip = devm_kzalloc(dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		sunxi_err(dev, "Fail to get IORESOURCE_MEM\n");
		return -EINVAL;
	}

	chip->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(chip->base)) {
		sunxi_err(dev, "Fail to map IO resource\n");
		return PTR_ERR(chip->base);
	}

	chip->dev = dev;

	hosc_clock = devm_clk_get(chip->dev, "hosc");
	if (!IS_ERR(hosc_clock)) {
		chip->hosc_fre = clk_get_rate(hosc_clock);
		if (chip->hosc_fre == 0) {
			sunxi_err(dev, "The clock frequency cannot be 0");
			return -EPERM;
		}
	}

	proc_create_single_data(DRV_NAME, 0, NULL, timestamp_proc_show, chip);

	return 0;
}

static int sunxi_timestamp_remove(struct platform_device *pdev)
{
	sunxi_info(NULL, "timestamp_dev remove");
	remove_proc_entry(DRV_NAME, NULL);
	return 0;
}

static const struct of_device_id sunxi_timestamp[] = {
	{ .compatible = "allwinner,timestamp"},
	{ /* Sentinel */},
};

static struct platform_driver timestamp_driver = {
	.probe  = sunxi_timestamp_probe,
	.remove = sunxi_timestamp_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = sunxi_timestamp,
	},
};

static int __init sunxi_timestamp_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&timestamp_driver);
	return ret;
}

static void __exit sunxi_timestamp_exit(void)
{
	platform_driver_unregister(&timestamp_driver);
}

module_init(sunxi_timestamp_init);
module_exit(sunxi_timestamp_exit);

MODULE_AUTHOR("chenbins<chenbins@allwinnertech.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
