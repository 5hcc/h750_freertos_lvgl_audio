#include "lv_port_disp.h"
#include "LCD\lcd.h"
#include "retarget.h"
#include "dma2d.h"
#include "mdma.h"

#define DISP_BUF_WIDTH   800
#define DISP_BUF_LINES   60
#define LCD_RAM_ADDR  0x60080000

static lv_disp_drv_t *g_disp_drv; 

static uint16_t disp_buf1[800 * 60]  __attribute__((aligned(32)));
static uint16_t disp_buf2[800 * 60]  __attribute__((aligned(32)));

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

void dma2d_xfer_cplt_cb(DMA2D_HandleTypeDef *hdma2d);
void mdma_xfer_cplt_cb(MDMA_HandleTypeDef *hmdma);
void mdma_xfer_error_cb(MDMA_HandleTypeDef *hmdma);

void lv_port_disp_init(void)
{
    static lv_disp_draw_buf_t draw_buf_dsc;
//    static lv_color_t buf_1[DISP_BUF_WIDTH * DISP_BUF_LINES];
	
    hdma2d.XferCpltCallback = dma2d_xfer_cplt_cb;		/*dma2d中断没有回调函数，自己创建一个函数连接到XferCpltCallback*/
//    hmdma_mdma_channel0_sw_0.XferBlockCpltCallback = mdma_xfer_cplt_cb;		/*mdma中断没有回调函数，自己创建一个函数连接到XferCpltCallback*/
//		hmdma_mdma_channel0_sw_0.XferErrorCallback = mdma_xfer_error_cb;
    lv_disp_draw_buf_init(&draw_buf_dsc, disp_buf1, disp_buf2, DISP_BUF_WIDTH * DISP_BUF_LINES);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    /* 分辨率取 lcd_init 识别到的实际值（0x5510->800x480，否则 480x800） */
    disp_drv.hor_res  = lcddev.width;
    disp_drv.ver_res  = lcddev.height;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf_dsc;

    lv_disp_drv_register(&disp_drv);
}

void dma2d_small_window_test(void)
{
    /* 条纹图案：每行前 20 像素红，其余蓝 —— 任何移位/陈旧都一眼可见 */
    for (int i =0; i < 224  * 122; i++) {
        int x = i % 224;
        disp_buf1[i] = (x < 20) ? 0xF800 : 0x001F;
    }

    lcd_set_window(289, 179, 289+223 , 300);     /* 就用失败过的那个窗口 */
    lcd_write_ram_prepare();

    SCB_CleanDCache_by_Addr((uint32_t *)disp_buf1, 224  * 122 * 2);

    /* 故意阻塞版：连中断/异步也一起排除 */
    HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)disp_buf1, LCD_RAM_ADDR, 224 , 122);
    HAL_DMA2D_PollForTransfer(&hdma2d, 1000);
}

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
	
	
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);
    lcd_set_window(area->x1, area->y1, area->x2, area->y2);
    lcd_write_ram_prepare();

    if(w % 8 == 0)
    {
        SCB_CleanDCache_by_Addr((uint32_t *)color_p, (int32_t)(w * h *2));
				g_disp_drv = disp_drv;
        HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)color_p, LCD_RAM_ADDR, w, h);
    }
    else
    {
        uint32_t n = w * h;
        while (n--) {
            LCD->LCD_RAM = color_p->full;
            color_p++;
        }
        lv_disp_flush_ready(disp_drv);
    }
			
//    SCB_CleanDCache_by_Addr((uint32_t *)color_p, (int32_t)(w * h *2));
//    g_disp_drv = disp_drv;
//		HAL_MDMA_Start_IT(&hmdma_mdma_channel0_sw_0, (uint32_t)color_p, LCD_RAM_ADDR, w * h * 2, 1);
}

void dma2d_xfer_cplt_cb(DMA2D_HandleTypeDef *hdma2d)
{
    lv_disp_flush_ready(g_disp_drv);
}

//void mdma_xfer_cplt_cb(MDMA_HandleTypeDef *hmdma)
//{
//	  lv_disp_flush_ready(g_disp_drv);
//}

//void mdma_xfer_error_cb(MDMA_HandleTypeDef *hmdma)
//{
//    printf("MDMA ERROR! ErrorCode=%lu CESR=0x%08lX\r\n",hmdma->ErrorCode, hmdma->Instance->CESR);
//}