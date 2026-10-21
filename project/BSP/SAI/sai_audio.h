#ifndef SAI_AUDIO_H
#define SAI_AUDIO_H

#include "AUDIO/audio_source.h"

/* 媒体项:统一描述任意来源的一段音频 */
typedef struct {
    const audio_source_t *src;
    const void           *param;   /* Flash 传 flash_audio_t*,U盘传路径 */
    const char           *name;
} media_item_t;

void audio_init(void);
void audio_play(const media_item_t *item);
void audio_stop(void);
void vaudiotask(void *pvParameters);

#endif