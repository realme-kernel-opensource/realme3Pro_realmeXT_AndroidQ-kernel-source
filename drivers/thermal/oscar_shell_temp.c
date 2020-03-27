/*
 * Copyright (C) 2019 OPPO, Inc.
 * Author: Fang Xiang <fangxiang@oppo.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) "%s:%s " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/thermal.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/spinlock.h>

enum {
	SHELL_FRONT = 0,
	SHELL_FRAME,
	SHELL_BACK,
	SHELL_MAX,
};

static DEFINE_IDA(shell_temp_ida);
static DEFINE_SPINLOCK(oscar_lock);
static int shell_temp[SHELL_MAX];

struct oscar_shell_temp {
	struct thermal_zone_device *tzd;
	int shell_id;
};

static struct of_device_id oscar_shell_of_match[] = {
	{ .compatible = "oppo,shell-temp" },
	{},
};

static int oscar_get_shell_temp(struct thermal_zone_device *tz,
					int *temp)
{
	struct oscar_shell_temp *ost;

	if (!temp || !tz)
		return -EINVAL;

	ost = tz->devdata;
	*temp = shell_temp[ost->shell_id];
	return 0;
}

struct thermal_zone_device_ops shell_thermal_zone_ops = {
	.get_temp = oscar_get_shell_temp,
};

static int oscar_shell_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *dev_node = dev->of_node;
	struct thermal_zone_device *tz_dev;
	struct oscar_shell_temp *ost;
	int ret = 0;
	int result;

	if (!of_device_is_available(dev_node)) {
		pr_err("shell-temp dev not found\n");
		return -ENODEV;
	}

	ost = kzalloc(sizeof(struct oscar_shell_temp), GFP_KERNEL);
	if (!ost) {
		pr_err("alloc oscar mem failed\n");
		return -ENOMEM;
	}

	result = ida_simple_get(&shell_temp_ida, 0, 0, GFP_KERNEL);
	if (result < 0) {
		pr_err("genernal oscar id failed\n");
		ret = -EINVAL;
		goto err_free_mem;
	}

	ost->shell_id = result;

	tz_dev = thermal_zone_device_register(dev_node->name,
			0, 0, ost, &shell_thermal_zone_ops, NULL, 0, 0);
	if (IS_ERR_OR_NULL(tz_dev)) {
		pr_err("register thermal zone for shell failed\n");
		ret = -ENODEV;
		goto err_remove_id;
	}
	ost->tzd = tz_dev;

	platform_set_drvdata(pdev, ost);

	return 0;

err_remove_id:
	ida_simple_remove(&shell_temp_ida, result);
err_free_mem:
	kfree(ost);
	return ret;
}

static int oscar_shell_remove(struct platform_device *pdev)
{
	struct oscar_shell_temp *ost = platform_get_drvdata(pdev);

	if (ost) {
		platform_set_drvdata(pdev, NULL);
		thermal_zone_device_unregister(ost->tzd);
		kfree(ost);
	}

	return 0;
}

static struct platform_driver oscar_shell_platdrv = {
	.driver = {
		.name		= "oscar-shell-temp",
		.owner		= THIS_MODULE,
		.of_match_table = oscar_shell_of_match,
	},
	.probe	= oscar_shell_probe,
	.remove = oscar_shell_remove,
};

#define BUF_LEN		256
static ssize_t proc_shell_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *pos)
{
	int ret;
	int index, temp, len;
	char tmp[BUF_LEN + 1];
	unsigned long flags;


	if (count == 0)
		return 0;

	len = count > BUF_LEN ? BUF_LEN : count;

	ret = copy_from_user(tmp, buf, len);
	if (ret) {
		pr_err("copy_from_user failed, ret=%d\n", ret);
		return count;
	}

	if (tmp[len - 1] == '\n')
		tmp[len - 1] = '\0';
	else
		tmp[len] = '\0';

	ret = sscanf(tmp, "%d %d", &index, &temp);
	if (ret < 2) {
		pr_err("write failed, ret=%d\n", ret);
		return count;
	}

	if (index >= SHELL_MAX) {
		pr_err("write invalid para\n");
		return count;
	}

	spin_lock_irqsave(&oscar_lock, flags);
	shell_temp[index] = temp;
	spin_unlock_irqrestore(&oscar_lock, flags);

	return count;
}

static const struct file_operations proc_shell_fops = {
	.write = proc_shell_write,
};

static int __init oscar_shell_init(void)
{
	struct proc_dir_entry *shell_proc_entry;

	shell_proc_entry = proc_create("shell-temp", 0666, NULL, &proc_shell_fops);
	if (!shell_proc_entry) {
		pr_err("shell-temp proc create failed\n");
		return -EINVAL;
	}

	spin_lock_init(&oscar_lock);

	return platform_driver_register(&oscar_shell_platdrv);
}

static void __exit oscar_shell_exit(void)
{
	platform_driver_unregister(&oscar_shell_platdrv);
}


module_init(oscar_shell_init);
module_exit(oscar_shell_exit);
