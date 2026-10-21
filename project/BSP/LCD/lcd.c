#include "lcd.h"
#include "lcd_init_ex.c" 
#include <stdio.h>

extern UART_HandleTypeDef huart3;

_lcd_dev lcddev;

void lcd_wr_regno(uint16_t regno)
{
	LCD->LCD_REG = regno;
}

void lcd_wr_data(uint16_t data)
{
	LCD->LCD_RAM = data;
}

void lcd_wr_reg(uint16_t regno , uint16_t data)
{
	LCD->LCD_REG = regno;
	LCD->LCD_RAM = data;
}

void lcd_write_ram_prepare(void)
{
	lcd_wr_regno(lcddev.wramcmd);   /* 0x2C00: 开始写 GRAM */
}

uint16_t lcd_rd_data(void)
{
	volatile uint16_t data;
	delay_us(100);
	data = LCD->LCD_RAM;
	return data;
}

uint8_t lcd_get_id()
{
	lcd_wr_reg(0xF000, 0x0055);
	lcd_wr_reg(0xF001, 0x00AA);
	lcd_wr_reg(0xF002, 0x0052);
	lcd_wr_reg(0xF003, 0x0008);
	lcd_wr_reg(0xF004, 0x0001);

	lcd_wr_regno(0xc500);
	lcddev.id = lcd_rd_data();
	lcddev.id <<= 8;

	lcd_wr_regno(0xc501);
	lcddev.id |= lcd_rd_data();

	delay_ms(5);
}

void lcd_gpio_init()
{
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void LCD_ConfigDev(uint16_t dir)
{
    lcddev.dir = dir;
    uint16_t regval = 0;
    if(lcddev.id == 0x5510)
    {
        lcddev.wramcmd = 0x2C00;
        lcddev.setxcmd = 0x2A00;
        lcddev.setycmd = 0x2B00;
    }

    switch (dir)
    {
        case 0:  lcddev.madctl = 0x0000; break;  // 竖屏
        case 1:  lcddev.madctl = 0x00A0; break;  // 横屏 (MY|MX|MV, 与正点原子一致)
        case 2:  lcddev.madctl = 0x00C0; break;  // 竖屏倒置
        case 3:  lcddev.madctl = 0x0020; break;  // 横屏倒置
        default: lcddev.madctl = 0x0000; break;
    }

    /* 根据 MV 位决定宽高 */
    if (lcddev.madctl & 0x20)   // bit5 = 1，交换 XY，横屏
    {
        lcddev.width  = 800;
        lcddev.height = 480;
    }
    else
    {
        lcddev.width  = 480;
        lcddev.height = 800;
    }

	lcd_wr_reg(0x3600, lcddev.madctl);

}

int lcd_init()
{
	lcd_gpio_init();
	lcd_get_id();
	

	lcd_nt35510_init();
	LCD_ConfigDev(1);
	LCD_BL(1);

}	

void lcd_set_window(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    /* NT35510 的 MV 位是行列交换: 横屏(MV=1)时 0x2A 直接对应长边 0~799,
       X/Y 范围不需要交换 */
    /* 列地址范围 */
    lcd_wr_regno(lcddev.setxcmd);       // 0x2A00
    lcd_wr_data(xs >> 8);
    lcd_wr_regno(lcddev.setxcmd + 1);   // 0x2A01
    lcd_wr_data(xs & 0xFF);
    lcd_wr_regno(lcddev.setxcmd + 2);   // 0x2A02
    lcd_wr_data(xe >> 8);
    lcd_wr_regno(lcddev.setxcmd + 3);   // 0x2A03
    lcd_wr_data(xe & 0xFF);

    /* 页地址范围 */
    lcd_wr_regno(lcddev.setycmd);       // 0x2B00
    lcd_wr_data(ys >> 8);
    lcd_wr_regno(lcddev.setycmd + 1);   // 0x2B01
    lcd_wr_data(ys & 0xFF);
    lcd_wr_regno(lcddev.setycmd + 2);   // 0x2B02
    lcd_wr_data(ye >> 8);
    lcd_wr_regno(lcddev.setycmd + 3);   // 0x2B03
    lcd_wr_data(ye & 0xFF);
}

void lcd_set_cursor(uint16_t x, uint16_t y)
{
    lcd_set_window(x, y, x, y);   // 单点

}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
	lcd_set_cursor(x, y);
	lcd_wr_reg(lcddev.wramcmd, color);
}

void lcd_clear(uint16_t color)
{
    uint32_t total = (uint32_t)lcddev.width * lcddev.height;

    lcd_set_window(0, 0, lcddev.width - 1, lcddev.height - 1);  // 全屏
    lcd_wr_regno(lcddev.wramcmd);   // 0x2C00

    for (uint32_t i = 0; i < total; i++)
    {
        lcd_wr_data(color);
    }
}

static uint16_t lcd_rd_reg(uint16_t regno)
{
    lcd_wr_regno(regno);
    (void)lcd_rd_data();   /* dummy read */
    return lcd_rd_data();
}

void lcd_debug_dump(void)
{
    char buf[96];
    int n;

    n = sprintf(buf, "id=%04X w=%u h=%u madctl=%04X\r\n",
                lcddev.id, lcddev.width, lcddev.height, lcddev.madctl);
    HAL_UART_Transmit(&huart3, (uint8_t *)buf, n, 100);

    lcd_set_window(0, 0, lcddev.width - 1, lcddev.height - 1);

    n = sprintf(buf, "2A: %02X %02X %02X %02X\r\n",
                lcd_rd_reg(0x2A00), lcd_rd_reg(0x2A01),
                lcd_rd_reg(0x2A02), lcd_rd_reg(0x2A03));
    HAL_UART_Transmit(&huart3, (uint8_t *)buf, n, 100);

    n = sprintf(buf, "2B: %02X %02X %02X %02X\r\n",
                lcd_rd_reg(0x2B00), lcd_rd_reg(0x2B01),
                lcd_rd_reg(0x2B02), lcd_rd_reg(0x2B03));
    HAL_UART_Transmit(&huart3, (uint8_t *)buf, n, 100);

    n = sprintf(buf, "36: %02X\r\n", lcd_rd_reg(0x3600));
    HAL_UART_Transmit(&huart3, (uint8_t *)buf, n, 100);
}
