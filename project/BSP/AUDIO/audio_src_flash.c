#include "audio_src_flash.h"
#include "mdma.h"

#define AUDIO_FLASH_BASE 0x90000000UL
#define AUDIO_FLASH_BYTES 0x00800000UL

extern MDMA_HandleTypeDef hmdma_mdma_channel0_sw_0;

static uint32_t flash_addr;
static uint32_t flash_size;

static uint8_t flash_open(const void *param, uint32_t *total_out)
{
    const flash_audio_t *song = (const flash_audio_t *)param;

    if ((song == NULL) || (total_out == NULL) || (song->bytes == 0U) ||
        ((song->addr | song->bytes) & 3U) != 0U ||
        (song->addr >= AUDIO_FLASH_BYTES) ||
        (song->bytes > AUDIO_FLASH_BYTES - song->addr)) {
        return 1;
    }
    flash_addr = AUDIO_FLASH_BASE + song->addr;
    flash_size = song->bytes;
    *total_out = flash_size;
    return 0;
}

static uint8_t flash_read(uint32_t offset, uint8_t *dst, uint32_t len)
{
    if ((dst == NULL) || (((uint32_t)dst | offset | len) & 3U) != 0U ||
        (offset > flash_size) || (len > flash_size - offset)) {
        return 1;
    }

    /* MDMA 直接写 SRAM,先作废 D-Cache 里对应区域的脏行 */
    if (len == 0U) {
        return 0;
    }
    SCB_InvalidateDCache_by_Addr((uint32_t *)dst, (int32_t)len);

    if(HAL_MDMA_Start(&hmdma_mdma_channel0_sw_0, flash_addr + offset, (uint32_t)dst, len, 1) != HAL_OK) {
        return 1;
    }
    if (HAL_MDMA_PollForTransfer(&hmdma_mdma_channel0_sw_0, HAL_MDMA_FULL_TRANSFER, 100) != HAL_OK) {
        HAL_MDMA_Abort(&hmdma_mdma_channel0_sw_0);   /* 超时自愈,通道拉回 READY */
        return 1;
    }
    SCB_InvalidateDCache_by_Addr((uint32_t *)dst, (int32_t)len);
    return 0;
}

static void flash_close(void)
{
}

const audio_source_t audio_src_flash = { flash_open, flash_read, flash_close };
