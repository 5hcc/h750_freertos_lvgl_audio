#ifndef __AUDIO_SOURCE_H
#define __AUDIO_SOURCE_H

#include "main.h"
#include <stdint.h>

typedef struct
{
    uint8_t(*open)(const void *param, uint32_t *total_out);     /* param 定位信息(由各实现定义类型), total_out: 打开成功回填总字节数 */
    uint8_t(*read)(uint32_t offset, uint8_t *buf, uint32_t len);
    void(*close)(void);
} audio_source_t;

#endif /* __AUDIO_SOURCE_H */