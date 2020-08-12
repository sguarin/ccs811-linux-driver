// SPDX-License-Identifier: GPL-2.0+
/*
 * ccs811.c - Support for the CCS811 air quality sensor
 *
 * Copyright (C) 2015, 2018
 * Author: Sebastián Guarino <sebastian.guarino@gmail.com>
 *
 * Datasheets:
 * https://www.sciosense.com/wp-content/uploads/2020/01/CCS811-Datasheet.pdf

 */

#define pr_fmt(fmt) "ccs811: " fmt

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>

#include "ccs811.h"

#define NAME "ccs811"


// File operations hooks
static int ccs811_open(struct inode *inode, struct file *file)  {
	pr_info("my_dev_open() fue invocada.\n");
	return 0;
}

static int ccs811_close(struct inode *inode, struct file *file)  {
	pr_info("my_dev_close() fue invocada.\n");
	return 0;
}

static long ccs811_ioctl(struct file *file, unsigned int cmd, unsigned long arg)  {
	pr_info("my_dev_ioctl() fue invocada. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

static ssize_t ccs811_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)  {
	pr_info("my_dev_write() fue invocada.");
	return 0;
}

static ssize_t ccs811_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)  {
	char output[32];
	size_t output_size;
	unsigned int cco;
	unsigned int tvoc;
	int ret;

	pr_info("my_dev_read() fue invocada.");

//	ret = ccs811_reg_read(ccs811.client, CCS811_REG_STATUS, &val);
//	pr_info("read status: returns=%d value=%X\n", ret, val);
//
	ret = ccs811_data_read(&cco, &tvoc);
	if (!ret) {
		pr_info("read: %u\n", cco);
		pr_info("read: %u\n", tvoc);
	}

	output_size = snprintf(output, 31, "CCO=%u PPM, TVOC=%u PPB\n", cco, tvoc);
	// TODO CHECK BOUNDARIES AND KEEP STATE OF OUTPUT
	copy_to_user(buffer, output, output_size);

	return output_size;
}

/* declaracion de una estructura del tipo file_operations */
static const struct file_operations ccs811_fops = {
	.owner = THIS_MODULE,
	.open = ccs811_open,
	.read = ccs811_read,
	.write = ccs811_write,
	.release = ccs811_close,
	.unlocked_ioctl = ccs811_ioctl,
};

/* declaracion e inicializacion de una estructura miscdevice */
static struct miscdevice ccs811_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = NAME,
	.fops = &ccs811_fops,
};

static int ccs811_probe(struct i2c_client *client, const struct i2c_device_id *id)  {
	int ret;
	unsigned int val;

	pr_info("Ejecutando funcion probe()\n");
	pr_info("Informacion de dispositivo conectado (struct i2c_client):\n");
	pr_info("\tDireccion: %X\n",client->addr);
	pr_info("\tNombre: %s\n", client->name);
	pr_info("\tDriver: %s\n", (client->dev).driver->name);

	pr_info("\n\nInformacion desde ID (struct i2c_device_id):\n");
	pr_info("\tNombre: %s", id->name);

	/* Registro del dispositivo con el kernel */
	ret = misc_register(&ccs811_miscdevice);
	if (ret != 0) {
		pr_err("No se pudo registrar el dispositivo\n");
		return ret;
	}

	//	pr_info("minor asignado: %i\n", ccs811_miscdevice.minor);
	//	pr_info("Informacion de dispositivo conectado luego de registrar:\n");
	//	pr_info("\tClase: %s\n", (client->dev).class->name);
	//	pr_info("\tMajor number: %d\n", MAJOR((client->dev).devt));

	ret = ccs811_start(client);
	if (!ret)
		pr_info("driver started succesfully\n");

	return 0;
}

static int ccs811_remove(struct i2c_client *client)  {

	pr_info("Ejecutando funcion remove()\n");

	/* Unregister del miscDevice del kernel */
	misc_deregister(&ccs811_miscdevice);
	//ccs811_end();
	pr_info("Modulo descargado, anulado el registro");
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
//MODULE_INFO(mse_imd, "Esto no es para simples mortales");
