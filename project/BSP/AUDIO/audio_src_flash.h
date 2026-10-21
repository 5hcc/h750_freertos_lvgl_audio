#ifndef AUDIO_SRC_FLASH_H
#define AUDIO_SRC_FLASH_H

#include "audio_source.h"

/* Flash 音源的定位参数:起始地址 + 字节数 */
typedef struct {
    uint32_t addr;
    uint32_t bytes;
} flash_audio_t;

extern const audio_source_t audio_src_flash;

#endif