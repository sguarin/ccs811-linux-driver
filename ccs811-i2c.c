// SPDX-License-Identifier: GPL-2.0
/*
 * ccs811-i2c.c - Support for the CCS811 air quality sensor
 *
 * Copyright (C) 2015, 2018
 * Author: Sebastián Guarino <sebastian.guarino@gmail.com>
 *
 * Datasheets:
 * https://www.sciosense.com/wp-content/uploads/2020/01/CCS811-Datasheet.pdf
 */

#define pr_fmt(fmt) "ccs811: " fmt

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "ccs811.h"

#define NAME "ccs811"
#define SET_MODE 100
#define GET_MODE 101

#define MODE	1

static bool debug = false;
module_param(debug, bool, 0600);
MODULE_PARM_DESC(debug, "enable debugging, dumping packets to KERN_DEBUG.");

static long ccs811_ioctl(struct file *file, unsigned int cmd, unsigned long arg)  {
	unsigned int mode;
	int ret;
	if (debug)
		pr_info("ccs811_ioctl() cmd = %d, arg = %ld\n", cmd, arg);

	switch(cmd){
	case SET_MODE:
		copy_from_user(&mode, (char *)arg, sizeof(mode));
		ccs811_set_mode(mode);
		break;
	case GET_MODE:
		mode = ccs811_get_mode();
		ret = copy_to_user((int*)arg, &mode, sizeof(mode));
		if (ret)
			pr_err("Error copy_to_user()");
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}

static ssize_t ccs811_write(struct file *file, const char __user *ubuf, size_t len, loff_t *offset)  {
	int test = 0;
	char kbuf[30];

	memset(kbuf, '\0', (size_t)30);

	if (copy_from_user(&kbuf, ubuf, len)) {
		return -EFAULT;
	}

	pr_info("my_dev_write() fue invocada test=%d.", test);
	return 0;
}

static ssize_t ccs811_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)  {
	char output[32];
	size_t output_size;
	unsigned int co2, tvoc;
	int ret;

	ret = ccs811_data_read(&co2, &tvoc);
	if (ret) {
		pr_err("Error reading ccs811 data\n");
		return -EIO;
	}

	if (debug) {
		pr_info("ccs811_data_read() returns CO2=%u TVOC=%u\n", co2, tvoc);
	}

	output_size = snprintf(output, 31, "CO2=%u PPM, TVOC=%u PPB\n", co2, tvoc);
	ret = copy_to_user(buffer, output, output_size);
	if (ret) {
		pr_err("Error copy_to_user()");
	}
	return output_size;
}

static const struct file_operations ccs811_fops = {
	.owner = THIS_MODULE,
	.read = ccs811_read,
	.unlocked_ioctl = ccs811_ioctl,
};

static struct miscdevice ccs811_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = NAME,
	.fops = &ccs811_fops,
};

static int ccs811_probe(struct i2c_client *client, const struct i2c_device_id *id)  {
	int ret;
	unsigned int val;

	if (debug) {
		pr_info("Running ccs811_probe()\n");
		pr_info("Device detected address=%X name=%s driver=%s\n", client->addr, client->name, (client->dev).driver->name);
	}

	/* register misc device */
	ret = misc_register(&ccs811_miscdevice);
	if (ret != 0) {
		pr_err("cannot register misc device\n");
		return ret;
	}

	ret = ccs811_start(client);
	if (debug)
		ccs811_set_debug(true);
	if (!ret && debug)
		pr_info("driver started succesfully\n");

	return 0;
}

static int ccs811_remove(struct i2c_client *client)  {

	/* Unregister del miscDevice del kernel */
	misc_deregister(&ccs811_miscdevice);
	//ccs811_end();
	return 0;
}


static const struct of_device_id ccs811_dt_ids[] = {
	{ .compatible = "ams,ccs811", },
	{ }
};
MODULE_DEVICE_TABLE(of, ccs811_dt_ids);

static struct i2c_driver ccs811_driver = {
	.probe= ccs811_probe,
	.remove= ccs811_remove,
	.driver = {
		.name = NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ccs811_dt_ids),
	},
};
module_i2c_driver(ccs811_driver);

MODULE_AUTHOR("Sebastian Guarino <sebastian.guarino@gmail.com>");
MODULE_DESCRIPTION("AMS CCS811 air quality sensor driver");
MODULE_LICENSE("GPL v2");

