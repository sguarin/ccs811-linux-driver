// SPDX-License-Identifier: GPL-2.0+
#define pr_fmt(fmt) "ccs811-core: " fmt

#include <linux/device.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/delay.h>

#include "ccs811.h"

struct ccs811_data {
	struct i2c_client *client;
	bool debug;
	//struct mutex lock;
};
static struct ccs811_data ccs811;

int ccs811_reg_read(struct ccs811_data *ccs811, uint8_t reg, unsigned int *val);
int ccs811_reg_write(struct ccs811_data *ccs811, uint8_t buf[]);

static bool ccs811_is_writeable_reg(uint8_t reg);
static bool ccs811_is_readable_reg(uint8_t reg);
static uint8_t ccs811_register_size(uint8_t reg);

static bool ccs811_is_writeable_reg(uint8_t reg)
{
	switch (reg) {
	case CCS811_REG_MEAS_MODE:
	case CCS811_REG_ENV_DATA:
	case CCS811_REG_THRESHOLDS:
	case CCS811_REG_BASELINE:
	case CCS811_REG_SW_RESET:
	case CCS811_REG_APP_ERASE:
	case CCS811_REG_APP_DATA:
	case CCS811_REG_APP_VERIFY:
	case CCS811_REG_APP_START:
		return true;
	default:
		return false;
	};
}

static bool ccs811_is_readable_reg(uint8_t reg)
{
	switch (reg) {
	case CCS811_REG_STATUS:
	case CCS811_REG_MEAS_MODE:
	case CCS811_REG_ALG_RESULT_DATA:
	case CCS811_REG_RAW_DATA:
	case CCS811_REG_BASELINE:
	case CCS811_REG_HW_ID:
	case CCS811_REG_HW_VERSION:
	case CCS811_REG_FW_BOOT_VERSION:
	case CCS811_REG_FW_APP_VERSION:
	case CCS811_REG_ERROR_ID:
		return true;
	default:
		return false;
	};
}

static uint8_t ccs811_register_size(uint8_t reg)
{
	switch (reg) {
	case CCS811_REG_STATUS:
	case CCS811_REG_MEAS_MODE:
	case CCS811_REG_HW_ID:
	case CCS811_REG_HW_VERSION:
	case CCS811_REG_INTERNAL_STATE:
	case CCS811_REG_ERROR_ID:
		return 1;
	case CCS811_REG_RAW_DATA:
	case CCS811_REG_BASELINE:
	case CCS811_REG_FW_BOOT_VERSION:
	case CCS811_REG_FW_APP_VERSION:
		return 2;
	case CCS811_REG_ENV_DATA:
	case CCS811_REG_THRESHOLDS:
	case CCS811_REG_SW_RESET:
	case CCS811_REG_APP_ERASE:
		return 4;
	case CCS811_REG_ALG_RESULT_DATA:
		return 8;
	case CCS811_REG_APP_DATA:
		return 9;
	default:
		return 0;
	};
}

int ccs811_data_read(unsigned int *pcco, unsigned int *ptvoc) {
	uint8_t reg = CCS811_REG_ALG_RESULT_DATA;
	int ret;
	int bytes_to_read;
	char buf[9] = {0,0,0,0,0,0,0,0,0};

	ret = i2c_master_send(ccs811.client, &reg, 1);
	if (ret != 1)
		return -EIO;

	bytes_to_read = ccs811_register_size(reg);
	ret = i2c_master_recv(ccs811.client, buf, bytes_to_read);
	if (ret != bytes_to_read)
		return -EIO;

	if (ccs811.debug)
		pr_info("DATA %X %X %X %X\n", buf[0], buf[1], buf[2], buf[3]);
	*pcco = buf[0]*256 + buf[1];
	*ptvoc = buf[2]*256 + buf[3];

	return 0;
}
EXPORT_SYMBOL(ccs811_data_read);

int ccs811_soft_reset(struct ccs811_data *ccs811) {
	int ret;
	int bytes_to_write;
	char buf[5] = {CCS811_REG_SW_RESET, 0x11, 0xE5, 0x72, 0x8A};

	bytes_to_write = 5;
	if (bytes_to_write > 0) {
			ret = i2c_master_send(ccs811->client, (char *)buf, bytes_to_write);
			if (ret != 5)
				return -EIO;
	}

	return 0;
}

int ccs811_start(struct i2c_client *client) {
	int ret;
	char buf[CCS811_REG_MAXLEN + 2];
	unsigned val;

	ccs811.client = client;
	//ccs811.debug = false;

	val = 0;
	ret = ccs811_soft_reset(&ccs811);
	if (ccs811.debug)
		pr_info("write reset: returns=%d\n", ret);
	udelay(CCS811_WAIT_AFTER_RESET_US);

//	ret = ccs811_reg_write(client, CCS811_REG_SW_RESET, &val);
//	pr_info("write reset: returns=%d\n", ret);

	ret = ccs811_reg_read(&ccs811, CCS811_REG_HW_ID, &val);
	if (ret || val != CCS811_HW_ID) {
		return -EINVAL;
	}

	ret = ccs811_reg_read(&ccs811, CCS811_REG_HW_VERSION, &val);
	if (ccs811.debug)
		pr_info("read hardware version: returns=%d value=%X\n", ret, val);
	ret = ccs811_reg_read(&ccs811, CCS811_REG_STATUS, &val);
	if (ccs811.debug)
		pr_info("read status: returns=%d value=%X\n", ret, val);
	if (!CCS811_STATUS_GET_APP_VALID(val)) {
		pr_err("status error: app firmware not loaded\n");
		return -EINVAL;
	}
	if (CCS811_STATUS_GET_ERROR(val)) {
		pr_warn("status error: previous error detected\n");
		ret = ccs811_reg_read(&ccs811, CCS811_REG_ERROR_ID, &val);
		pr_warn("read error id: returns=%d value=%x\n", ret, val);
	}

	ret = ccs811_reg_read(&ccs811, CCS811_REG_FW_BOOT_VERSION, &val);
	if (ccs811.debug)
		pr_info("read firmware boot version: returns=%d value=%X\n", ret, val);
	ret = ccs811_reg_read(&ccs811, CCS811_REG_FW_APP_VERSION, &val);
	if (ccs811.debug)
		pr_info("read firmware app version: returns=%d value=%X\n", ret, val);

	val = 0;
	buf[0] = CCS811_REG_APP_START;
	buf[1] = '\0';
	ret = ccs811_reg_write(&ccs811, buf);
	if (ccs811.debug)
		pr_info("write app start: returns=%d\n", ret);
	udelay(CCS811_WAIT_AFTER_APPSTART_US);

	ret = ccs811_reg_read(&ccs811, CCS811_REG_STATUS, &val);
	if (ccs811.debug)
		pr_info("read status: returns=%d value=%X\n", ret, val);

	ret = ccs811_set_mode(1);

	return ret;
}
EXPORT_SYMBOL(ccs811_start);

void ccs811_set_debug(bool debug) {
	ccs811.debug = debug;
}
EXPORT_SYMBOL(ccs811_set_debug);

int ccs811_set_mode(unsigned int mode) {
	int ret;
	char buf[CCS811_REG_MAXLEN + 2];
	unsigned int val;

	ret = ccs811_reg_read(&ccs811, CCS811_REG_MEAS_MODE, &val);
	if (ccs811.debug)
		pr_info("read measure mode: returns=%d value=%X\n", ret, val);

	buf[0] = CCS811_REG_MEAS_MODE;
	buf[1] = CCS811_MEAS_MODE_SET_MODE(0, mode);
	ret = ccs811_reg_write(&ccs811, buf);
	if (ccs811.debug)
		pr_info("write measure mode: value=%X returns=%d\n", val, ret);

	ret = ccs811_reg_read(&ccs811, CCS811_REG_MEAS_MODE, &val);
	if (ccs811.debug)
		pr_info("read measure mode: returns=%d value=%X\n", ret, val);
	if (mode != CCS811_MEAS_MODE_GET_MODE(val))
		return -EIO;
	return 0;
}
EXPORT_SYMBOL(ccs811_set_mode);

int ccs811_get_mode(void) {
	int ret;
	unsigned int val;

	ret = ccs811_reg_read(&ccs811, CCS811_REG_MEAS_MODE, &val);
	if (ccs811.debug)
		pr_info("read measure mode: returns=%d value=%X\n", ret, val);
	if (ret <0)
		return -EIO;
	val = CCS811_MEAS_MODE_GET_MODE(val);
	return val;
}
EXPORT_SYMBOL(ccs811_get_mode);

int ccs811_reg_read(struct ccs811_data *ccs811, uint8_t reg, unsigned int *pval) {
	int ret;
	int bytes_to_read;

	if (!ccs811_is_readable_reg(reg))
		return -EINVAL;

	ret = i2c_master_send(ccs811->client, &reg, 1);
	if (ret != 1)
		return -EIO;

	*pval = 0;
	bytes_to_read = ccs811_register_size(reg);
	ret = i2c_master_recv(ccs811->client, (char *)pval, bytes_to_read);
	if (ret != bytes_to_read)
		return -EIO;
	return 0;
}
EXPORT_SYMBOL(ccs811_reg_read);

int ccs811_reg_write(struct ccs811_data *ccs811, uint8_t *buf) {
	int ret;
	int bytes_to_write;

	if (!ccs811_is_writeable_reg(buf[0]))
		return -EINVAL;

	bytes_to_write = ccs811_register_size(buf[0]) + 1;
	ret = i2c_master_send(ccs811->client, (char *)buf, bytes_to_write);
	if (ret != bytes_to_write)
		return -EIO;
	return 0;
}
