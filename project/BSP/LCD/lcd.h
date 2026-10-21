#ifndef __LCD_H__
#define __LCD_H__

#include "main.h"
#include "DELAY/delay.h"


#define LCD_BL_GPIO_PORT            GPIOB
#define LCD_BL_GPIO_PIN             GPIO_PIN_5
#define LCD_BL_GPIO_CLK_ENABLE()    do { __HAL_RCC_GPIOB_CLK_ENABLE(); } while (0)


#define LCD_BL(x)                   do { (x) ?                                                                  \
                                        HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, GPIO_PIN_SET):     \
                                        HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, GPIO_PIN_RESET);   \
                                    } while (0)

#define LCD_FMC_NEX 	1 
#define LCD_FMC_AX  	18

typedef struct
{
	volatile uint16_t LCD_REG;
	volatile uint16_t LCD_RAM;
}LCD_Typedef;

typedef struct
{
	uint16_t id;
	uint16_t dir;    //0,竖屏;1,横屏
	uint16_t wramcmd;   // 写显存命令
	uint16_t setxcmd;   // 设置 X 坐标命令
	uint16_t setycmd;   // 设置 Y 坐标命令
	uint16_t width  ;   // 屏幕宽度
	uint16_t height ;   // 屏幕高度
	uint16_t madctl;   // 显示控制寄存器  0x0000竖屏 / 0x00E0横屏 / 0x00C0竖屏倒置 / 0x0020横屏倒置
}_lcd_dev;

extern _lcd_dev lcddev;

/* 颜色定义 */
#define WHITE                       0xFFFF  /* 白色 */
#define BLACK                       0x0000  /* 黑色 */
#define RED                         0xF800  /* 红色 */
#define GREEN                       0x07E0  /* 绿色 */
#define BLUE                        0x001F  /* 蓝色 */
#define MAGENTA                     0xF81F  /* 品红色 */
#define YELLOW                      0xFFE0  /* 黄色 */
#define CYAN                        0x07FF  /* 青色 */
#define BROWN                       0xBC40  /* 棕色 */
#define BRRED                       0xFC07  /* 棕红色 */
#define GRAY                        0x8430  /* 灰色 */
#define DARKBLUE                    0x01CF  /* 深蓝色 */
#define LIGHTBLUE                   0x7D7C  /* 浅蓝色 */
#define GRAYBLUE                    0x5458  /* 灰蓝色 */
#define LIGHTGREEN                  0x841F  /* 浅绿色 */
#define LGRAY                       0xC618  /* 浅灰色 */
#define LGRAYBLUE                   0xA651  /* 浅灰蓝色 */
#define LBBLUE                      0x2B12  /* 浅棕蓝色 */


#define LCD_BASE			(uint32_t)((0x60000000 + (0x4000000 * (LCD_FMC_NEX - 1))) | (((1 << LCD_FMC_AX) * 2) - 2))
#define LCD						((LCD_Typedef *) LCD_BASE)		

#define ATK_MD0430_USING_TOUCH 1

int lcd_init(void);
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_clear(uint16_t color);
void lcd_debug_dump(void);
void lcd_wr_regno(uint16_t regno);
void lcd_set_window(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void lcd_write_ram_prepare(void);

#endif