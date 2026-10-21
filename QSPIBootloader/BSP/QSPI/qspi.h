#ifndef __QSPI_H
#define __QSPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "quadspi.h"
#include <stdio.h>
#include <string.h>

/* ============================================================
 * W25Q64 参数
 * ============================================================ */
#define W25QXX_SECTOR_SIZE      4096                /* 扇区 4KB */
#define W25QXX_PAGE_SIZE        256                 /* 页 256 字节 */
#define W25QXX_FLASH_SIZE       (8 * 1024 * 1024)   /* 8MB */

/* ============================================================
 * W25Q64 指令
 * ============================================================ */
#define W25QXX_CMD_WRITE_ENABLE     0x06
#define W25QXX_CMD_WRITE_DISABLE    0x04
#define W25QXX_CMD_READ_STATUS      0x05
#define W25QXX_CMD_WRITE_STATUS     0x01
#define W25QXX_CMD_READ_ID          0x9F
#define W25QXX_CMD_READ_DATA        0x03
#define W25QXX_CMD_FAST_READ        0x0B
#define W25QXX_CMD_QUAD_READ        0xEB
#define W25QXX_CMD_PAGE_PROGRAM     0x02
#define W25QXX_CMD_SECTOR_ERASE     0x20
#define W25QXX_CMD_BLOCK_ERASE      0xD8
#define W25QXX_CMD_CHIP_ERASE       0xC7

/* ============================================================
 * W25Q64 预期 ID
 * ============================================================ */
#define W25QXX_ID_W25Q64        0xEF40
#define W25QXX_ID_W25Q128       0xEF18

/* ============================================================
 * 对外接口
 * ============================================================ */
uint16_t W25QXX_ReadID(void);
void     W25QXX_WriteEnable(void);
void     W25QXX_WaitBusy(void);
void     W25QXX_EraseSector(uint32_t addr);
void     W25QXX_WritePage(uint32_t addr, uint8_t *buf, uint16_t len);
void     W25QXX_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void     W25QXX_EnableMemoryMapped(void);
void     W25QXX_Test(void);
void     W25QXX_Test2(void);
void     W25QXX_Test3(void);
void     W25QXX_Test4(void);

#ifdef __cplusplus
}
#endif

#endif /* __QSPI_H */