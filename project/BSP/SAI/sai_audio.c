#include "sai_audio.h"
#include "sai.h"
#include "ES8388/es8388.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

extern SAI_HandleTypeDef hsai_BlockA1;

#define AUDIO_BUF_SIZE    4096                    /* 半边 4096 个 16bit 采样 */
#define AUDIO_BUF_BYTES   (AUDIO_BUF_SIZE * 2)

static uint16_t audio_ring[AUDIO_BUF_SIZE * 2] __attribute__((aligned(32)));

static TaskHandle_t          audio_task_handle;
static const audio_source_t *cur_source;
static volatile uint32_t     audio_offset;
static volatile uint32_t     audio_total;
static volatile uint8_t      audio_playing;
static uint32_t              audio_half_bytes[2];

/* ---- SAI 中断:只发通知,通知值 = 空出的半缓冲编号 ---- */
static void audio_notify(uint32_t half)
{
    BaseType_t woken = pdFALSE;

    if (audio_task_handle != NULL) {
        xTaskNotifyFromISR(audio_task_handle, 1UL << half, eSetBits, &woken);
        portYIELD_FROM_ISR(woken);
    }
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &hsai_BlockA1) audio_notify(0);
}
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &hsai_BlockA1) audio_notify(1);
}

/* ---- 填充一半,失败不推进 offset,下次通知自然重试 ---- */
static uint8_t audio_fill(uint32_t half)
{
    uint8_t *dst = (uint8_t *)&audio_ring[half * AUDIO_BUF_SIZE];
    uint32_t remaining = audio_total - audio_offset;
    uint32_t len = (remaining < AUDIO_BUF_BYTES) ? remaining : AUDIO_BUF_BYTES;

    audio_half_bytes[half] = 0U;
    if (len < AUDIO_BUF_BYTES) {
        memset(dst, 0, AUDIO_BUF_BYTES);
        SCB_CleanDCache_by_Addr((uint32_t *)dst, AUDIO_BUF_BYTES);
    }
    if (len == 0U) {
        return 0;
    }
    if (cur_source->read(audio_offset, dst, len) != 0U) {
        memset(dst, 0, AUDIO_BUF_BYTES);
        SCB_CleanDCache_by_Addr((uint32_t *)dst, AUDIO_BUF_BYTES);
        return 1;
    }
    SCB_CleanDCache_by_Addr((uint32_t *)dst, AUDIO_BUF_BYTES);
    audio_offset += len;
    audio_half_bytes[half] = len;
    return 0;
}

void audio_init(void)
{
    es8388_init();
    es8388_sai_cfg(0, 3);      /* I2S, 16bit */
    es8388_adda_cfg(1, 0);     /* 开 */
    es8388_output_cfg(1, 1);   /* 开输出通道 */
    es8388_spkvol_set(10);
}

void audio_play(const media_item_t *item)
{
    uint32_t total;

    if ((item == NULL) || (item->src == NULL)) return;
    audio_stop();
    if (item->src->open(item->param, &total) != 0U) return;

    cur_source = item->src;
    audio_total = total;
    audio_offset = 0U;
    audio_half_bytes[0] = 0U;
    audio_half_bytes[1] = 0U;
    audio_playing = 1U;

    if ((total == 0U) || (audio_fill(0) != 0U) || (audio_fill(1) != 0U)) {
        audio_stop();
        return;
    }
    if (HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audio_ring,
                             AUDIO_BUF_SIZE * 2) != HAL_OK) {
        audio_stop();
    }
}

void audio_stop(void)
{
    if (audio_playing != 0U) {
        audio_playing = 0U;
        HAL_SAI_DMAStop(&hsai_BlockA1);
        cur_source->close();
        cur_source = NULL;
    }
}

/* ---- 常驻音频任务 ---- */
void vaudiotask(void *pvParameters)
{
    uint32_t ready;
    uint32_t half;

    audio_task_handle = xTaskGetCurrentTaskHandle();
    audio_init();
    if (pvParameters != NULL) {
        audio_play((const media_item_t *)pvParameters);
    }

    for (;;) {
        xTaskNotifyWait(0, UINT32_MAX, &ready, portMAX_DELAY);
        for (half = 0U; half < 2U; ++half) {
            if ((ready & (1UL << half)) == 0U || audio_playing == 0U) continue;
            if ((audio_offset == audio_total) &&
                (audio_half_bytes[half] != 0U) &&
                (audio_half_bytes[half ^ 1U] == 0U)) {
                audio_stop();
            } else if (audio_fill(half) != 0U) {
                audio_stop();
            }
        }
    }
}