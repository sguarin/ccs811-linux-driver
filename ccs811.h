/*
 * ccs811.h
 *
 *  Created on: Aug 10, 2020
 *      Author: sguarin
 */

#ifndef CCS811_H_
#define CCS811_H_

/* Common registers */
#define CCS811_REG_STATUS           0x00
#define CCS811_REG_HW_ID            0x20
#define CCS811_REG_HW_VERSION       0x21
#define CCS811_REG_FW_BOOT_VERSION  0x23 // 2 bytes
#define CCS811_REG_FW_APP_VERSION   0x24 // 2 bytes
#define CCS811_REG_ERROR_ID         0xE0
#define CCS811_REG_SW_RESET         0xFF // 4 bytes

/* Application specific registers */
#define CCS811_REG_MEAS_MODE        0x01
#define CCS811_REG_ALG_RESULT_DATA  0x02 // up to 8 bytes
#define CCS811_REG_RAW_DATA         0x03 // 2 bytes
#define CCS811_REG_ENV_DATA         0x05 // 4 bytes
#define CCS811_REG_THRESHOLDS       0x10 // 5 bytes
#define CCS811_REG_BASELINE         0x11 // 2 bytes
#define CCS811_REG_INTERNAL_STATE   0xA0

/* Bootloader specific registers */
#define CCS811_REG_APP_ERASE        0xF1 // 4 bytes
#define CCS811_REG_APP_DATA         0xF2 // 9 bytes
#define CCS811_REG_APP_VERIFY       0xF3 // 0 bytes
#define CCS811_REG_APP_START        0xF4 // 0 bytes

#define CCS811_REG_MAXLEN			9

/* CCS811 constant register values */
#define CCS811_HW_ID				0x81

/* STATUS register flags */
#define CCS811_STATUS_GET_ERROR(r)		(r && 0x01)
#define CCS811_STATUS_GET_APP_VALID(r)	(r && 0x10)

/* MEAS_MODE register flags */
#define CCS811_MEAS_MODE_GET_MODE(r)	(r >> 4 & 0x07)
#define CCS811_MEAS_MODE_SET_MODE(r, v)	(v << 4 | r)


/* Timings */
#define CCS811_WAIT_AFTER_RESET_US     2000 // The CCS811 needs a wait after reset
#define CCS811_WAIT_AFTER_APPSTART_US  1000 // The CCS811 needs a wait after app start
#define CCS811_WAIT_AFTER_WAKE_US        50 // The CCS811 needs a wait after WAKE signal
#define CCS811_WAIT_AFTER_APPERASE_MS   500 // The CCS811 needs a wait after app erase (300ms from spec not enough)
#define CCS811_WAIT_AFTER_APPVERIFY_MS   70 // The CCS811 needs a wait after app verify
#define CCS811_WAIT_AFTER_APPDATA_MS     50 // The CCS811 needs a wait after writing app data

/* ERROR_ID register flags */
//#define CCS811_ERROR_ID_

int ccs811_start(struct i2c_client *client);
int ccs811_set_mode(unsigned int mode);
int ccs811_get_mode(void);
void ccs811_set_debug(bool debug);
int ccs811_data_read(unsigned int *pcco, unsigned int *ptvoc);

#endif /* CCS811_H_ */
