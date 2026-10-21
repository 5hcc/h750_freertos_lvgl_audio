#ifndef __MYI2C_H__
#define	__MYI2C_H__

#include "main.h"
#include "DELAY\delay.h"
#include "retarget.h"

#define I2C_SCL_GPIO_PORT           GPIOH
#define I2C_SCL_GPIO_PIN            GPIO_PIN_4
#define I2C_SCL_GPIO_CLK_ENABLE()   do { __HAL_RCC_GPIOH_CLK_ENABLE(); } while (0)
#define I2C_SDA_GPIO_PORT           GPIOH
#define I2C_SDA_GPIO_PIN            GPIO_PIN_5
#define I2C_SDA_GPIO_CLK_ENABLE()   do { __HAL_RCC_GPIOH_CLK_ENABLE(); } while (0)

#define I2C_SCL(x)                  do { (x) ?                                                                  \
                                        HAL_GPIO_WritePin(I2C_SCL_GPIO_PORT, I2C_SCL_GPIO_PIN, GPIO_PIN_SET):   \
                                        HAL_GPIO_WritePin(I2C_SCL_GPIO_PORT, I2C_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                                    } while (0)
#define I2C_SDA(x)                  do { (x) ?                                                                  \
                                        HAL_GPIO_WritePin(I2C_SDA_GPIO_PORT, I2C_SDA_GPIO_PIN, GPIO_PIN_SET):   \
                                        HAL_GPIO_WritePin(I2C_SDA_GPIO_PORT, I2C_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                                    } while (0)
#define I2C_SDA_READ                ((HAL_GPIO_ReadPin(I2C_SDA_GPIO_PORT, I2C_SDA_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1)


void i2c_init();
void i2c_start(void);
void i2c_stop(void);
uint8_t i2c_wait_ack(void);
void i2c_send_byte(uint8_t data);
uint8_t i2c_read_byte(uint8_t ack);																		
uint8_t i2c_wr_reg(uint8_t reg , uint8_t val);																		
uint8_t i2c_rd_reg(uint8_t reg);
uint8_t i2c_test_comm(void);
#endif