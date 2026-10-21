#include "myi2c.h"

void i2c_init()
{
	  GPIO_InitTypeDef gpiostruct = {0};

		I2C_SCL_GPIO_CLK_ENABLE();
    I2C_SDA_GPIO_CLK_ENABLE();
		
		gpiostruct.Pin = I2C_SCL_GPIO_PIN;
		gpiostruct.Mode = GPIO_MODE_OUTPUT_PP;
		gpiostruct.Pull = GPIO_PULLUP;
		gpiostruct.Speed = GPIO_SPEED_FREQ_HIGH;
		
		HAL_GPIO_Init(I2C_SCL_GPIO_PORT, &gpiostruct);
		
		gpiostruct.Pin = I2C_SDA_GPIO_PIN;
		gpiostruct.Mode = GPIO_MODE_OUTPUT_OD;

		HAL_GPIO_Init(I2C_SDA_GPIO_PORT, &gpiostruct);

}

void i2c_start(void)
{
    I2C_SDA(1);
    I2C_SCL(1);
    delay_us(2);
    I2C_SDA(0);
    delay_us(2);
    I2C_SCL(0);
    delay_us(2);
}

void i2c_stop(void)
{
    I2C_SDA(0);
    delay_us(2);
    I2C_SCL(1);
    delay_us(2);
    I2C_SDA(1);
    delay_us(2);
}

uint8_t i2c_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;
    
    I2C_SDA(1);
    delay_us(2);
    I2C_SCL(1);
    delay_us(2);
    
    while (I2C_SDA_READ != 0)
    {
        waittime++;
        if (waittime > 250)
        {
            i2c_stop();
            rack = 1;
            break;
        }
    }
    
    I2C_SCL(0);
    delay_us(2);
    
    return rack;
}

void i2c_ack(void)
{
    I2C_SDA(0);
    delay_us(2);
    I2C_SCL(1);
    delay_us(2);
    I2C_SCL(0);
    delay_us(2);
    I2C_SDA(1);
    delay_us(2);
}

void i2c_nack(void)
{
    I2C_SDA(1);
    delay_us(2);
    I2C_SCL(1);
    delay_us(2);
    I2C_SCL(0);
    delay_us(2);
}


void i2c_send_byte(uint8_t data)
{
    uint8_t t;
    
    for (t=0; t<8; t++)
    {
        I2C_SDA((data & 0x80) >> 7);
        delay_us(2);
        I2C_SCL(1);
        delay_us(2);
        I2C_SCL(0);
        data <<= 1;
    }
    
    I2C_SDA(1);
}

uint8_t i2c_read_byte(uint8_t ack)
{
    uint8_t i;
    uint8_t receive = 0;
    
    for (i=0; i<8; i++)
    {
        receive <<= 1;
        I2C_SCL(1);
        delay_us(2);
        
        if (I2C_SDA_READ)
        {
            receive++;
        }
        
        I2C_SCL(0);
        delay_us(2);
    }
    
    if (ack == 0)
    {
        i2c_nack();
    }
    else
    {
        i2c_ack();
    }
    
    return receive;
}

uint8_t i2c_wr_reg(uint8_t reg, uint8_t val)
{
    i2c_start();

    i2c_send_byte((0x10 << 1) | 0);   /* 地址+写 */
    if (i2c_wait_ack()) {
        i2c_stop();
        return 1;
    }

    i2c_send_byte(reg);                       /* 寄存器地址 */
    if (i2c_wait_ack()) {
        i2c_stop();
        return 2;
    }

    i2c_send_byte(val);                       /* 数据 */
    if (i2c_wait_ack()) {
        i2c_stop();
        return 3;
    }

    i2c_stop();
    return 0;
}

/* ========== 读寄存器 ========== */
uint8_t i2c_rd_reg(uint8_t reg)
{
    uint8_t temp = 0;

    i2c_start();

    i2c_send_byte((0x10 << 1) | 0);   /* 地址+写 */
    if (i2c_wait_ack()) {
        i2c_stop();
        return 0xFF;
    }

    i2c_send_byte(reg);                       /* 寄存器地址 */
    if (i2c_wait_ack()) {
        i2c_stop();
        return 0xFF;
    }

    i2c_start();
    i2c_send_byte((0x10 << 1) | 1);   /* 地址+读 */
    if (i2c_wait_ack()) {
        i2c_stop();
        return 0xFF;
    }

    temp = i2c_read_byte(0);                  /* 读数据，NACK */
    i2c_stop();

    return temp;
}

/* ========== 通信测试 ========== */
uint8_t i2c_test_comm(void)
{
    uint8_t wrdata = 0x55;
    uint8_t rddata = 0;
    uint8_t reg = 0x1A;    /* DACCONTROL4，可读写 */
		i2c_init();
    printf("\r\n===== ES8388 I2C Test =====\r\n");

    /* 写 */
    if (i2c_wr_reg(reg, wrdata) == 0) {
        printf("Write OK: reg=0x%02X, val=0x%02X\r\n", reg, wrdata);
    } else {
        printf("Write FAIL\r\n");
        return 1;
    }

    /* 读 */
    rddata = i2c_rd_reg(reg);
    printf("Read: 0x%02X\r\n", rddata);

    if (rddata == wrdata) {
        printf("ES8388 I2C Test PASS\r\n");
        return 0;
    } else {
        printf("ES8388 I2C Test FAIL\r\n");
        return 1;
    }
}