#ifndef __PCF8574_H__
#define __PCF8574_H__

#include "I2C/myi2c.h"

#define PCF8574_ADDR 0x20

/* 端口功能定义 */
#define BEEP_IO                         0
#define AP_INT_IO                       1
#define DCMI_PWDN_IO                    2
#define USB_PWR_IO                      3
#define MD_PD_IO                        4   /* 扬声器功放关断控制 */
#define MPU_INT_IO                      5
#define RS485_RE_IO                     6
#define ETH_RESET_IO                    7

void pcf8574_i2c_test();
void pcf8574_wr_byte(uint8_t data);
void pcf8574_write_bit(uint8_t bits, uint8_t bit);
#endif