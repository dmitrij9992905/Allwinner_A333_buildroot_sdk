// SPDX-License-Identifier: GPL-2.0+
/* Copyright(c) 2025 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner Sunxi SoCs Security ID support.
 */

//#define DEBUG
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/err.h>
#include <sunxi-smc.h>
#include <sunxi-sbi.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/acpi.h>
#include <sunxi-sid.h>
#include <sunxi-log.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include "sunxi-sid-v2.h"

#define SUNXI_SID_DEV_NAME	"sid_efuse"
#define SUNXI_SID_REG_CELLS	2  /* dts: '#address-cells + #size-cells' */
#define SUNXI_SID_KEY_CELLS	4  /* keyram size: <offset size shift mask> */
#define SUNXI_SID_REG_SIZE	4

static struct sunxi_sid_chip *sunxi_sid_chip;
static atomic_t sunxi_sid_dev_opened;

static int sunxi_get_key_info(char *name, struct sunxi_sid_key *key_info)
{
	u32 i = 0;
	u32 key_num;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	key_num = sunxi_sid_chip->dts.key_num;
	for (i = 0; i < key_num; i++) {
		if (strcmp(sunxi_sid_chip->dts.key_info[i].name, name))
			continue;

		strlcpy(key_info->name, name, sizeof(key_info->name));
		key_info->size = sunxi_sid_chip->dts.key_info[i].size;
		key_info->offset = sunxi_sid_chip->dts.key_info[i].offset;

		return 0;
	}

	sunxi_err(sunxi_sid_chip->dev, "Failed to read key '%s', key not support\n", name);
	return -EINVAL;
}

static int sunxi_keyram_read(char *name, u32 *buf)
{
	u32 i = 0, err = 0;
	struct sunxi_sid_key key_info;

	err = sunxi_get_key_info(name, &key_info);
	if (err)
		return err;

	for (i = 0; i < key_info.size; i += 4)
		buf[i / 4] = readl(sunxi_sid_chip->base + key_info.offset);

	return 0;
}

static int sunxi_get_key_data(char *name, u32 **buf, struct sunxi_sid_key *key_info)
{
	int err = 0;

	err = sunxi_get_key_info(name, key_info);
	if (err)
		goto err0;

	*buf = devm_kzalloc(sunxi_sid_chip->dev, key_info->size, GFP_KERNEL);
	if (!buf) {
		sunxi_err(sunxi_sid_chip->dev, "Failed to alloc '*buf'\n");
		err = -ENOMEM;
		goto err0;
	}

	err = sunxi_keyram_read(name, *buf);
	if (err)
		goto err1;

	return 0;

err1:
	devm_kfree(sunxi_sid_chip->dev, buf);
err0:
	return err;
}

static void sunxi_put_key_data(u32 *buf)
{
	devm_kfree(sunxi_sid_chip->dev, buf);
}

static int sunxi_get_key_one_word(char *name, u32 *data)
{
	struct sunxi_sid_key key_info;
	u32 *key_buf = NULL;
	u32 val;
	int err = 0;

	err = sunxi_get_key_data(name, &key_buf, &key_info);
	if (err)
		return err;

	val = key_buf[0] >> key_info.shift;
	val &= key_info.mask;
	*data = val;

	sunxi_put_key_data(key_buf);

	return 0;
}

unsigned int sunxi_get_soc_ver(void)
{
	u32 sunxi_soc_ver;
	const struct sunxi_sid_dts *dts;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	dts = &(sunxi_sid_chip->dts);
	sunxi_soc_ver = readl(dts->soc_ver_base);
	sunxi_soc_ver &= dts->soc_ver_mask;

	return sunxi_soc_ver;
}
EXPORT_SYMBOL(sunxi_get_soc_ver);

/* get die info like 1919 */
unsigned int sunxi_get_platform_id(void)
{
	const struct sunxi_sid_dts *dts;
	u32 val = 0;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return 0;
	}

	dts = &(sunxi_sid_chip->dts);
	val = readl(dts->soc_ver_base);
	val |= (1 << SUNXI_DIE_ENABLE_OFFSET);
	writel(val, dts->soc_ver_base);
	val = readl(dts->soc_ver_base);

	return val & SUNXI_DIE_INFO_MASK;
}
EXPORT_SYMBOL(sunxi_get_platform_id);

int sunxi_soc_is_secure(void)
{
	int err = 0;
	u32 val;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	if (sunxi_sid_chip->data->secure_status_offset) {
		return readl(sunxi_sid_chip->base + sunxi_sid_chip->data->secure_status_offset) & 0x1;
	}

	err = sunxi_get_key_one_word("secbit", &val);
	if (err)
		return err;

	return val;
}
EXPORT_SYMBOL(sunxi_soc_is_secure);

s32 sunxi_efuse_readn(s8 *key_name, void *buf, u32 n)
{
	u32 *key_buf = NULL;
	int err;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	if (!key_name || !*key_name || !n || !buf) {
		sunxi_err(sunxi_sid_chip->dev, "Invalid parameter\n");
		return -EINVAL;
	}

	if (n < 4) {
		sunxi_err(sunxi_sid_chip->dev, "Invalid parameter, the key size must >= 4 \n");
		return -EINVAL;
	}

	key_buf = devm_kzalloc(sunxi_sid_chip->dev, n, GFP_KERNEL);
	if (!key_buf) {
		sunxi_err(sunxi_sid_chip->dev, "Failed to malloc key_buf\n");
		err = -ENOMEM;
		goto err0;
	}

	err = sunxi_keyram_read(key_name, key_buf);
	if (err) {
		err = -EINVAL;
		goto err1;
	}

	memcpy(buf, key_buf, n);
	devm_kfree(sunxi_sid_chip->dev, key_buf);

	return 0;

err1:
	devm_kfree(sunxi_sid_chip->dev, key_buf);
err0:
	return err;
}
EXPORT_SYMBOL(sunxi_efuse_readn);

s32 sunxi_efuse_read(s8 *key_name, void *buf)
{
	int err = 0;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	if (!key_name || !buf) {
		sunxi_err(sunxi_sid_chip->dev, "Invalid parameter\n");
		return -EINVAL;
	}

	err = sunxi_keyram_read(key_name, buf);
	if (err)
		return -EINVAL;

	return 0;
}
EXPORT_SYMBOL(sunxi_efuse_read);

int sunxi_get_module_param_from_sid(u32 *dst, u32 offset, u32 len)
{
	u32 i = 0;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	if (!dst) {
		sunxi_err(sunxi_sid_chip->dev, "Invalid parameter: the dst buf is NULL\n");
		return -EINVAL;
	}

	if ((len & 0x3) || (offset & 0x3)) {
		sunxi_err(sunxi_sid_chip->dev, "Invalid parameter: the len and offset must word align\n");
		return -EINVAL;
	}

	for (i = 0; i < (len >> 2); i++)
		dst[i] = readl(sunxi_sid_chip->dts.key_base + offset + (i * 4));

	return 0;
}
EXPORT_SYMBOL(sunxi_get_module_param_from_sid);

int sunxi_get_soc_chipid(unsigned char *chipid)
{
	int err = 0;
	struct sunxi_sid_key key_info;
	u32 *sunxi_soc_chipid = NULL;

	err = sunxi_get_key_data("chipid", &sunxi_soc_chipid, &key_info);
	if (err)
		return err;

	memcpy(chipid, sunxi_soc_chipid, key_info.size);
	sunxi_put_key_data(sunxi_soc_chipid);
	return 0;
}
EXPORT_SYMBOL(sunxi_get_soc_chipid);

/* get chipid[16:127] for ic uuid */
int sunxi_get_serial(unsigned char *serial)
{
	int err = 0;
	u32 chipid_data[4] = {0};
	u32 sunxi_serial[4] = {0};

	err = sunxi_get_soc_chipid((unsigned char *)chipid_data);
	if (err)
		return err;

	sunxi_serial[0] = chipid_data[3];
	sunxi_serial[1] = chipid_data[2];
	sunxi_serial[2] = (chipid_data[1] >> 16) & 0x0FFFF;
	memcpy(serial, sunxi_serial, 16);

	return 0;
}
EXPORT_SYMBOL(sunxi_get_serial);

int sunxi_get_soc_chipid_str(char *serial)
{
	int err = 0;
	size_t size;
	u32 chipid_data[4] = {0};
	struct sunxi_sid_hw_data *data = NULL;

	if (!serial) {
		sunxi_err(NULL, "Invalid parameter: the chipid_origin is NULL\n");
		return -EINVAL;
	}

	err = sunxi_get_soc_chipid((unsigned char *)chipid_data);
	if (err)
		return err;

	data = sunxi_sid_chip->data;
	size = sprintf(serial, "%08x", chipid_data[0] & data->markid_mask);

	return size;
}
EXPORT_SYMBOL(sunxi_get_soc_chipid_str);

int sunxi_get_soc_chipid_origin(char *chipid_origin)
{
	if (!chipid_origin) {
		sunxi_err(NULL, "Invalid parameter: the chipid_origin is NULL\n");
		return -EINVAL;
	}

	return  sunxi_get_soc_chipid(chipid_origin);
}
EXPORT_SYMBOL(sunxi_get_soc_chipid_origin);

unsigned int sunxi_get_soc_markid(void)
{
	int err = 0;
	unsigned int val;
	u32 chipid_data[4] = {0};

	err = sunxi_get_soc_chipid((unsigned char *)chipid_data);
	if (err)
		return 0;

	val = chipid_data[0] & 0xffff;

	return val;
}
EXPORT_SYMBOL(sunxi_get_soc_markid);

int sunxi_sid_sram_read32(const char *key, u32 *data)
{
	int err = 0;
	u32 val;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	err = sunxi_get_key_one_word((char *)key, &val);
	if (err)
		return err;

	*data = val;

	return 0;
}
EXPORT_SYMBOL(sunxi_sid_sram_read32);

int sunxi_sid_get_ecc_status(void)
{
	u32 val = 0;
	int err = 0;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	err = sunxi_get_key_one_word("ecc", &val);
	if (err)
		return 0;

	return val;
}
EXPORT_SYMBOL_GPL(sunxi_sid_get_ecc_status);

int sunxi_get_soc_dvfs(u32 *dvfs)
{
	u32 dvfs_bak = 0, dvfs_ori = 0;
	int err;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	err = sunxi_get_key_one_word("dvfs_ori", &dvfs_ori);
	if (err)
		return err;

	err = sunxi_get_key_one_word("dvfs_bak", &dvfs_bak);
	if (err)
		return err;

	*dvfs = dvfs_bak ? dvfs_bak : dvfs_ori;

	return 0;
}
EXPORT_SYMBOL(sunxi_get_soc_dvfs);

int sunxi_get_sid_ver(u32 *ver)
{
	u32 sid_ver = 0;
	struct sunxi_sid_dts *dts_info;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	dts_info = &(sunxi_sid_chip->dts);
	if (!dts_info->sid_ver_offset) {
		sunxi_err(sunxi_sid_chip->dev, "Failed to get sid ver\n");
		return -EINVAL;
	}

	sid_ver = readl(sunxi_sid_chip->base + dts_info->sid_ver_offset);
	*ver = sid_ver;

	return 0;
}
EXPORT_SYMBOL(sunxi_get_sid_ver);

int sunxi_get_soc_ft_zone_str(char *serial)
{
	int err = 0;
	size_t size;
	struct sunxi_sid_key key_info;
	u32 *sunxi_soc_ftzone = NULL;

	err = sunxi_get_key_data("ft_zone", &sunxi_soc_ftzone, &key_info);
	if (err)
		return err;

	size = sprintf(serial, "%08x", ((sunxi_soc_ftzone[0] >> key_info.offset) & key_info.mask));
	sunxi_put_key_data(sunxi_soc_ftzone);

	return size;
}
EXPORT_SYMBOL(sunxi_get_soc_ft_zone_str);

int sunxi_get_soc_rotpk_status_str(char *status)
{
	uint32_t rotpk_status;
	struct sunxi_sid_hw_data *hw_data;

	if (!sunxi_sid_chip) {
		sunxi_err(NULL, "sunxi_sid_chip is NULL\n");
		return -EINVAL;
	}

	hw_data = sunxi_sid_chip->data;
	if (!hw_data->rotpk_status_offset) {
		sunxi_err(sunxi_sid_chip->dev, "Failed to get soc rotpk status\n");
		return -1;
	}

	rotpk_status = readl(sunxi_sid_chip->base + hw_data->rotpk_status_offset);
	rotpk_status &= hw_data->rotpk_status_mask;

	return sprintf(status, "%d", (rotpk_status) >> 1);
}
EXPORT_SYMBOL(sunxi_get_soc_rotpk_status_str);

s32 sunxi_get_platform(s8 *buf, s32 size)
{
	return snprintf(buf, size, "%s", CONFIG_AW_SOC_NAME);
}
EXPORT_SYMBOL(sunxi_get_platform);

unsigned int sunxi_get_soc_bin(void)
{
	u32 val, sunxi_soc_bin = 0;
	u32 err = 0;

	err = sunxi_get_key_one_word("soc_bin", &val);
	if (err)
		return 0;

	switch (val) {
	case 0b000001:
		sunxi_soc_bin = 1;
		break;
	case 0b000011:
		sunxi_soc_bin = 2;
		break;
	case 0b000111:
		sunxi_soc_bin = 3;
		break;
	default:
		break;
	}

	return sunxi_soc_bin;
}
EXPORT_SYMBOL(sunxi_get_soc_bin);

static int sunxi_efuse_set_power(struct sunxi_sid_chip *chip, bool enable)
{
	if (!chip->regulator)
		return 0;

	if (enable)
		return regulator_enable(chip->regulator);
	else
		return regulator_disable(chip->regulator);
}

static int sunxi_efuse_ioctl_read(struct sunxi_sid_chip *chip, sunxi_efuse_key_info_t *user_key)
{
	int err = 0;

	if (user_key->offset <= chip->dts.key_nonsec_offset_max) {
		err = sunxi_keyram_read(user_key->name, (u32 *)chip->user_data);
		if (err) {
			sunxi_err(chip->dev, "Failed to read key '%s' from efuse\n", user_key->name);
			goto err;
		}
	}
#if IS_ENABLED(CONFIG_AW_SMC)
	else {
		err = arm_svc_efuse_read(virt_to_phys((const volatile void *)user_key->name),
						virt_to_phys((const volatile void *)chip->user_data));
	}
#elif IS_ENABLED(CONFIG_AW_SBI)
	return err; /* TODO: support and add sbi_efuse_read() */
#endif

err:
	return err;
}

static int sunxi_efuse_ioctl_write(struct sunxi_sid_chip *chip, sunxi_efuse_key_info_t *key_info)
{
	int ret = 0;

	sunxi_efuse_set_power(chip, true);

#if IS_ENABLED(CONFIG_AW_SMC)
	ret = arm_svc_efuse_write(virt_to_phys((volatile void *)key_info));
#elif IS_ENABLED(CONFIG_AW_SBI)
	ret = sbi_efuse_write(virt_to_phys((volatile void *)key_info));
#else
	sunxi_err("Failed to write efuse, the platform not support\n");
	ret = -1;
#endif

	sunxi_efuse_set_power(chip, false);
	return ret;
}

static long sunxi_efuse_ioctl(struct file *file, unsigned int ioctl_num,
			      unsigned long ioctl_param)
{
	int err = 0;
	sunxi_efuse_key_info_t *key_info;
	struct sunxi_sid_chip *chip;
	uintptr_t key_data;

	chip = file->private_data;
	if (!chip) {
		printk("Get chip failed\n");
		return -EFAULT;
	}

	mutex_lock(&chip->mutex);
	key_info = &chip->user_key;
	if (copy_from_user(key_info, (void __user *)ioctl_param,
			   sizeof(*key_info))) {
		sunxi_err(chip->dev, "Failed to copy from user\n");
		err = -EFAULT;
		goto err0;
	}

	if (key_info->len > chip->dts.key_size_max) {
		sunxi_err(chip->dev, "User key_info->len = %d is larger than key max size %d\n", key_info->len, chip->dts.key_size_max);
		err = -EFAULT;
		goto err0;
	}

	if (key_info->offset > EFUSE_MAX_ADDR_SIZE) {
		sunxi_err(chip->dev, "User key_info->offset = %d is larger than key max offset\n", key_info->len);
		err = -EFAULT;
		goto err0;
	}

	if (key_info->offset + key_info->len > EFUSE_MAX_ADDR_SIZE) {
		sunxi_err(chip->dev, "the key size is larger than max size %x\n", EFUSE_MAX_ADDR_SIZE);
		err = -EFAULT;
		goto err0;
	}

	switch (ioctl_num) {
	case SUNXI_EFUSE_READ:
		err = sunxi_efuse_ioctl_read(chip, key_info);
		if (err < 0) {
			sunxi_err(chip->dev, "Failed to read key '%s' from efuse\n", key_info->name);
			err = -EFAULT;
			goto err1;
		}
		key_data = (uintptr_t)key_info->key_data;
		if (copy_to_user((void __user *)key_data, chip->user_data, key_info->len)) {
			sunxi_err(chip->dev, "Failed to copy to user\n");
			err = -EFAULT;
			goto err1;
		}
		break;
	case SUNXI_EFUSE_WRITE:
		err = copy_from_user(chip->user_data, (void __user *)(unsigned long)key_info->key_data, key_info->len);
		if (err) {
			sunxi_err(chip->dev, "Failed to copy data from user\n");
			err = -EFAULT;
			goto err1;
		}
		key_info->key_data = virt_to_phys((void *)chip->user_data);
		err = sunxi_efuse_ioctl_write(chip, key_info);
		if (err <= 0) {
			sunxi_err(chip->dev, "Failed to write key '%s' to efuse\n", key_info->name);
			err = -EFAULT;
			goto err1;
		}
		break;
	default:
		sunxi_err(chip->dev, "Unsupport ioctl cmd %d\n", ioctl_num);
		err = -EFAULT;
		break;
	}

err1:
	memset(chip->user_data, 0, chip->dts.key_size_max);
err0:
	mutex_unlock(&chip->mutex);

	return err;
}

static int sunxi_efuse_open(struct inode *inode, struct file *file)
{
	struct sunxi_sid_dev_info *dev_info;
	struct miscdevice *miscdev;
	int ret = 0;

	/* Only allow one instance */
	ret = atomic_cmpxchg(&sunxi_sid_dev_opened, 0, 1);
	if (ret)
		return -EBUSY;

	miscdev = file->private_data;
	dev_info = container_of(miscdev, struct sunxi_sid_dev_info, miscdev);
	file->private_data = dev_info->chip;

	return 0;
}

static int sunxi_efuse_close(struct inode *inode, struct file *file)
{
	atomic_set(&sunxi_sid_dev_opened, 0);
	return 0;
}

static int sunxi_sid_dts_get(struct sunxi_sid_chip *chip)
{
	struct device_node *np = chip->of_node;
	struct sunxi_sid_dts *dts = &chip->dts;
	struct resource res;
	struct device_node *node = NULL;
	struct device_node *child_node = NULL;
	int i = 0, err = 0;
	u32 *nonsec_keys_info = NULL;
	u32 key_reg[SUNXI_SID_REG_CELLS];
	const char *temp_name;
	u32 key_base_phy, key_ram_size;

	child_node = of_get_child_by_name(np, "keyram");
	if (!child_node) {
		sunxi_err(chip->dev, "Failed to find child_node: keysram from dts\n");
		err = -EINVAL;
		goto err0;
	}

	err = of_property_read_u32_array(child_node, "reg", key_reg, SUNXI_SID_REG_CELLS);
	if (err) {
		sunxi_err(chip->dev, "Failed to read keysram reg from dts\n");
		goto err0;
	}
	dts->key_nonsec_adress_offset = key_reg[0];
	dts->key_nonsec_offset_max = key_reg[1];

	dts->key_num = of_property_count_strings(child_node, "non-sec-key-names");
	if (dts->key_num < 0) {
		sunxi_err(chip->dev, "Failed to count non-sec-key-names\n");
		err = -EINVAL;
		goto err0;
	}

	nonsec_keys_info = devm_kzalloc(chip->dev, dts->key_num * SUNXI_SID_KEY_CELLS * sizeof(*nonsec_keys_info), GFP_KERNEL);
	if (!nonsec_keys_info) {
		sunxi_err(chip->dev, "Failed to alloc non_sec_keys from dts\n");
		err = -ENOMEM;
		goto err0;
	}

	err = of_property_read_u32_array(child_node, "non-sec-keys", nonsec_keys_info, dts->key_num * SUNXI_SID_KEY_CELLS);
	if (err) {
		sunxi_err(chip->dev, "Failed to read non-sec-keys from dts\n");
		goto err1;
	}

	dts->key_info = devm_kzalloc(chip->dev, dts->key_num * sizeof(*dts->key_info), GFP_KERNEL);
	if (!dts->key_info) {
		sunxi_err(chip->dev, "Failed to alloc key_info\n");
		err = -ENOMEM;
		goto err1;
	}

	for (i = 0; i < dts->key_num; i++) {
		err = of_property_read_string_index(child_node, "non-sec-key-names", i, &temp_name);
		if (err) {
			sunxi_err(chip->dev, "Failed to read non-sec-keys-names from dts\n");
			goto err2;
		}
		strlcpy(dts->key_info[i].name, temp_name, sizeof(dts->key_info[i].name));
		dts->key_info[i].offset = nonsec_keys_info[i * SUNXI_SID_KEY_CELLS];
		dts->key_info[i].size = nonsec_keys_info[i * SUNXI_SID_KEY_CELLS + 1];
		dts->key_info[i].shift = nonsec_keys_info[i * SUNXI_SID_KEY_CELLS + 2];
		dts->key_info[i].mask = nonsec_keys_info[i * SUNXI_SID_KEY_CELLS + 3];
	}

	err = of_property_read_u32(child_node, "key-size-max", &dts->key_size_max);
	if (err) {
		sunxi_err(chip->dev, "Failed to read key-size-max from dts\n");
		goto err2;
	}

	key_base_phy = chip->res->start + dts->key_nonsec_adress_offset;
	key_ram_size = dts->key_nonsec_offset_max;
	dts->key_base = devm_ioremap(chip->dev, key_base_phy, key_ram_size);
	if (!dts->key_base) {
		sunxi_err(chip->dev, "Failed to ioremap key base address\n");
		err = -ENOMEM;
		goto err2;
	}

	node = of_parse_phandle(np, "soc-ver-handle", 0);
	if (!node) {
		sunxi_err(chip->dev, "Failed to parsed soc-ver-handle\n");
		err = -EINVAL;
		goto err3;
	}

	err = of_address_to_resource(node, 0, &res);
	if (err) {
		sunxi_err(chip->dev, "Failed to parsed soc-ver base address\n");
		goto err3;
	}

	child_node = of_get_child_by_name(node, "soc-ver");
	if (!child_node) {
		sunxi_err(chip->dev, "Failed to find soc-ver node\n");
		err = -EINVAL;
		goto err3;
	}

	err = of_property_read_u32(child_node, "mask", &dts->soc_ver_mask);
	if (err) {
		sunxi_err(chip->dev, "Failed to read soc-ver 'mask' from dts\n");
		goto err3;
	}

	err = of_property_read_u32(child_node, "offset", &dts->soc_ver_reg);
	if (err) {
		sunxi_err(chip->dev, "Failed to read soc-ver 'offset' from dts\n");
		goto err3;
	}

	dts->soc_ver_reg += res.start;
	dts->soc_ver_base = devm_ioremap(chip->dev, dts->soc_ver_reg, SUNXI_SID_REG_SIZE);
	if (!dts->soc_ver_base) {
		sunxi_err(chip->dev, "Failed to ioremap soc-ver base address\n");
		goto err3;
	}

	err = of_property_read_u32(np, "sid-ver-offset", &dts->sid_ver_offset);
	if (err)
		sunxi_info(chip->dev, "Failed to read sid-ver offset from dts");

	devm_kfree(chip->dev, nonsec_keys_info);
	return 0;

err3:
	devm_iounmap(chip->dev, dts->key_base);
err2:
	devm_kfree(chip->dev, dts->key_info);
err1:
	devm_kfree(chip->dev, nonsec_keys_info);
err0:
	return err;
}

static void sunxi_sid_dts_put(struct sunxi_sid_chip *chip)
{
	devm_iounmap(chip->dev, chip->dts.soc_ver_base);
	devm_iounmap(chip->dev, chip->dts.key_base);
	devm_kfree(chip->dev, chip->dts.key_info);
}

static int sunxi_sid_regulator_enable(struct sunxi_sid_chip *chip)
{
	int err = 0;
	u32 vol;

	chip->regulator = devm_regulator_get(chip->dev, "sid");
	if (!chip->regulator) {
		sunxi_err(chip->dev, "Failed to get efuse regulator\n");
		err = -ENOMEM;
		goto err0;
	}

	err = of_property_read_u32(chip->of_node, "voltage", &vol);
	if (err) {
		sunxi_info(chip->dev, "Failed to read efuse voltage\n");
		err = 0;
		goto err1;
	}

	err = regulator_set_voltage(chip->regulator, vol * 1000, vol * 1000);
	if (err) {
		sunxi_err(chip->dev, "Failed to set efuse voltage %d\n", vol);
		goto err1;
	}

	return 0;

err1:
	devm_regulator_put(chip->regulator);
err0:
	return err;
}

static int sunxi_sid_regulator_disable(struct sunxi_sid_chip *chip)
{
	devm_regulator_put(chip->regulator);
	return 0;
}

static int sunxi_sid_resource_get(struct sunxi_sid_chip *chip)
{
	int err = 0;

	chip->res = platform_get_resource(chip->pdev, IORESOURCE_MEM, 0);
	if (!chip->res) {
		sunxi_err(chip->dev, "Failed to get IORESOURCE_MEM\n");
		err = -ENODEV;
		goto err0;
	}

	chip->base = devm_ioremap_resource(chip->dev, chip->res);
	if (!chip->base) {
		sunxi_err(chip->dev, "Failed to ioremap sid base address\n");
		err = -ENOMEM;
		goto err0;
	}

	err = sunxi_sid_dts_get(chip);
	if (err)
		goto err1;

	err = sunxi_sid_regulator_enable(chip);
	if (err)
		goto err2;

	chip->user_data = devm_kzalloc(chip->dev, chip->dts.key_size_max, GFP_KERNEL);
	if (!chip->user_data) {
		sunxi_err(chip->dev, "Failed to kzalloc user_data\n");
		err = -ENOMEM;
		goto err3;
	}

	return 0;

err3:
	sunxi_sid_regulator_disable(chip);
err2:
	sunxi_sid_dts_put(chip);
err1:
	devm_iounmap(chip->dev, chip->base);
err0:
	return err;
}

static void sunxi_sid_resource_put(struct sunxi_sid_chip *chip)
{
	devm_kfree(chip->dev, chip->user_data);
	sunxi_sid_regulator_disable(chip);
	sunxi_sid_dts_put(chip);
	devm_iounmap(chip->dev, chip->base);
}

static const struct file_operations sunxi_efuse_ops = {
	.owner = THIS_MODULE,
	.open = sunxi_efuse_open,
	.release = sunxi_efuse_close,
	.unlocked_ioctl = sunxi_efuse_ioctl,
	.compat_ioctl = sunxi_efuse_ioctl,
};

static struct miscdevice sunxi_efuse_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = SUNXI_SID_DEV_NAME,
	.fops = &sunxi_efuse_ops
};

static const struct sunxi_sid_hw_data sunxi_sid_v201_data = {
	.markid_mask = 0xffff,
	.rotpk_status_offset = 0x140,
	.rotpk_status_mask = 0x3,
	.secure_status_offset = 0xa0,
};

static const struct of_device_id sunxi_sid_of_match[] = {
	{ .compatible = "allwinner,sunxi-sid-v201", .data = &sunxi_sid_v201_data },
	{/* sentinel */},
};
MODULE_DEVICE_TABLE(of, sunxi_sid_of_match);

static int sunxi_sid_probe(struct platform_device *pdev)
{
	struct sunxi_sid_chip *chip = NULL;
	struct sunxi_sid_dev_info *dev_info = NULL;
	const struct of_device_id *of_id;
	int err = 0;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip) {
		sunxi_err(&pdev->dev, "Failed to kzalloc chip\n");
		err = -ENOMEM;
		goto err0;
	}

	chip->pdev = pdev;
	chip->dev = &pdev->dev;
	chip->of_node = pdev->dev.of_node;

	of_id = of_match_device(sunxi_sid_of_match, chip->dev);
	if (!of_id) {
			sunxi_err(chip->dev, "of_match_device() failed\n");
			err = -EINVAL;
			goto err1;
	}
	chip->data = (struct sunxi_sid_hw_data *)(of_id->data);

	err = sunxi_sid_resource_get(chip);
	if (err)
		goto err1;

	platform_set_drvdata(pdev, chip);
	sunxi_sid_chip = chip;

	dev_info = devm_kzalloc(&pdev->dev, sizeof(*dev_info), GFP_KERNEL);
	if (!dev_info) {
		sunxi_err(&pdev->dev, "Failed to kzalloc chip\n");
		err = -ENOMEM;
		goto err2;
	}

	dev_info->miscdev = sunxi_efuse_device;
	dev_info->chip = chip;
	chip->dev_info = dev_info;
	err = misc_register(&dev_info->miscdev);
	if (err) {
		sunxi_err(chip->dev, "Failed to register miscdev of sid\n");
		goto err3;
	}

	return 0;

err3:
	devm_kfree(chip->dev, dev_info);
err2:
	sunxi_sid_resource_put(chip);
err1:
	devm_kfree(chip->dev, chip);
err0:
	return err;
}

static int sunxi_sid_remove(struct platform_device *pdev)
{
	struct sunxi_sid_chip *chip = platform_get_drvdata(pdev);
	devm_kfree(chip->dev, chip->dev_info);
	misc_deregister(&sunxi_efuse_device);
	sunxi_sid_resource_put(chip);
	devm_kfree(chip->dev, chip);
	return 0;
}

static struct platform_driver sunxi_sid_driver = {
	.probe  = sunxi_sid_probe,
	.remove = sunxi_sid_remove,
	.driver = {
		.name = "sunxi-sid-v2",
		.of_match_table = sunxi_sid_of_match,
	},
};
module_platform_driver(sunxi_sid_driver);

MODULE_AUTHOR("zhoujie <zhoujie@allwinnertech.com>");
MODULE_DESCRIPTION("Allwinner security id driver v2");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");
