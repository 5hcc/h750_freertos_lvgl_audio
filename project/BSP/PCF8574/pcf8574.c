#include "pcf8574.h"

void pcf8574_wr_byte(uint8_t data)
{
    i2c_start();
    i2c_send_byte(PCF8574_ADDR << 1);
    i2c_wait_ack();
    i2c_send_byte(data);
    i2c_wait_ack();
    i2c_stop();
}

uint8_t pcf8574_rd_byte(void)
{
    uint8_t byte;

    i2c_start();
    i2c_send_byte((PCF8574_ADDR << 1) | 0x01);
    i2c_wait_ack();
    byte = i2c_read_byte(0);
    i2c_stop();

    return byte;
}

void pcf8574_write_bit(uint8_t bits, uint8_t bit)
{
    uint8_t byte = pcf8574_rd_byte();

    if (bit == 0) {
        byte &= ~(1 << bits);
    } else {
        byte |= (1 << bits);
    }

    pcf8574_wr_byte(byte);
}


void pcf8574_i2c_test()
{
    uint8_t wrdata = 0x55;
    uint8_t rddata = 0;
    uint8_t reg = 0x1A;    /* DACCONTROL4，可读写 */
        i2c_init();
    
    printf("\r\n===== PCF8574 I2C Test =====\r\n");
    
    /* 写 */
    if (i2c_wr_reg(reg, wrdata) == 0) {
        printf("Write 0x%02X to register 0x%02X successfully.\r\n", wrdata, reg);
    } else {
        printf("Failed to write to register 0x%02X.\r\n", reg);
        return;
    }
    
    /* 读 */
    rddata = i2c_rd_reg(reg);
    if (rddata != 0xFF) {
        printf("Read 0x%02X from register 0x%02X successfully.\r\n", rddata, reg);
    } else {
        printf("Failed to read from register 0x%02X.\r\n", reg);
        return;
    }
    
    /* 比较 */
    if (rddata == wrdata) {
        printf("I2C communication test passed!\r\n");
    } else {
        printf("I2C communication test failed! Written: 0x%02X, Read: 0x%02X\r\n", wrdata, rddata);
    }
}