#include "qspi.h"
#include "DELAY\delay.h"

extern UART_HandleTypeDef huart3;

int fputc(int ch, FILE *f)
{
    
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

/**
 * @brief  QSPI 通用命令
 * @param  instr    指令
 * @param  addr     地址（0 表示无地址）
 * @param  addr_mode 地址模式（QSPI_ADDRESS_NONE / 1_LINE / 4_LINES）
 * @param  data_mode 数据模式（QSPI_DATA_NONE / 1_LINE / 4_LINES）
 * @param  dummy    Dummy cycles
 * @param  buf      数据缓冲区（发送或接收）
 * @param  len      数据长度
 * @param  is_read  1=读，0=写
 */
static HAL_StatusTypeDef QSPI_Command(uint8_t instr, uint32_t addr,
                                      uint32_t addr_mode, uint32_t data_mode,
                                      uint8_t dummy, uint8_t *buf,
                                      uint32_t len, uint8_t is_read)
{
    QSPI_CommandTypeDef cmd = {0};

    cmd.Instruction     = instr;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Address         = addr;
    cmd.AddressMode     = addr_mode;
    cmd.AddressSize     = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode        = data_mode;
    cmd.NbData          = len;
    cmd.DummyCycles     = dummy;
    cmd.DdrMode         = QSPI_DDR_MODE_DISABLE;
    cmd.SIOOMode        = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        return HAL_ERROR;
    }

    if (data_mode != QSPI_DATA_NONE && len > 0) {
        if (is_read) {
            return HAL_QSPI_Receive(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
        } else {
            return HAL_QSPI_Transmit(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE);
        }
    }

    return HAL_OK;
}

void W25QXX_WaitBusy(void)
{
    uint8_t status = 0;
    do {
        QSPI_Command(0x05, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE,
                     0, &status, 1, 1);
    } while (status & 0x01);   /* BUSY 位 */
}

uint16_t W25QXX_ReadID(void)
{
    uint8_t id[3];
    QSPI_Command(0x9F, 0, QSPI_ADDRESS_NONE, QSPI_DATA_1_LINE, 0, id, 3, 1);
    return (id[0] << 8) | id[1];
}

void W25QXX_WriteEnable(void)
{
    QSPI_Command(0x06, 0, QSPI_ADDRESS_NONE, QSPI_DATA_NONE, 0, NULL, 0, 0);
}

void W25QXX_EraseSector(uint32_t addr)
{
    W25QXX_WriteEnable();
    QSPI_Command(0x20, addr, QSPI_ADDRESS_1_LINE, QSPI_DATA_NONE, 0, NULL, 0, 0);
    W25QXX_WaitBusy();
}

void W25QXX_WritePage(uint32_t addr, uint8_t *buf, uint16_t len)
{
    W25QXX_WriteEnable();
    QSPI_Command(0x02, addr, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, 0, buf, len, 0);
    W25QXX_WaitBusy();
}

void W25QXX_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    QSPI_Command(0x03, addr, QSPI_ADDRESS_1_LINE, QSPI_DATA_1_LINE, 0, buf, len, 1);
}


void W25QXX_EnableMemoryMapped(void)
{
    /* 先禁用 QSPI */
    QUADSPI->CR &= ~QUADSPI_CR_EN;
    while (QUADSPI->CR & QUADSPI_CR_EN);

    /* 配置 DCR */
    QUADSPI->DCR = (23 << QUADSPI_DCR_FSIZE_Pos)
                 | (3  << QUADSPI_DCR_CSHT_Pos)
                 | (0  << QUADSPI_DCR_CKMODE_Pos);

    /* 配置 CCR */
    QUADSPI->CCR = (0xEB << QUADSPI_CCR_INSTRUCTION_Pos)
                 | (1    << QUADSPI_CCR_IMODE_Pos)
                 | (3    << QUADSPI_CCR_ADMODE_Pos)
                 | (2    << QUADSPI_CCR_ADSIZE_Pos)
                 | (0    << QUADSPI_CCR_ABMODE_Pos)
                 | (3    << QUADSPI_CCR_DMODE_Pos)
                 | (6    << QUADSPI_CCR_DCYC_Pos)
                 | (3    << QUADSPI_CCR_FMODE_Pos);

    /* DLR = 0, ABR = 0 */
    QUADSPI->DLR = 0;
    QUADSPI->ABR = 0;

    /* 使能 QSPI */
    QUADSPI->CR |= QUADSPI_CR_EN;
}

void W25QXX_Test(void)
{
    uint16_t id;
    uint8_t  wbuf[32] = "Hello W25Q64!";
    uint8_t  rbuf[32] = {0};

    /* 读 ID */
    id = W25QXX_ReadID();
    printf("ID = 0x%04X (expect 0xEF40)\r\n", id);

    /* 擦除 + 写 + 读 */
    W25QXX_EraseSector(0x000000);
    W25QXX_WritePage(0x000000, wbuf, strlen((char *)wbuf));
    W25QXX_Read(0x000000, rbuf, strlen((char *)wbuf));
    printf("Read back: %s\r\n", rbuf);

    if (memcmp(wbuf, rbuf, strlen((char *)wbuf)) == 0) {
        printf("QSPI Test PASS\r\n");
    } else {
        printf("QSPI Test FAIL\r\n");
    }
}

void W25QXX_Test2(void)
{
    uint8_t  wbuf[32] = "Mapped Mode OK";
    uint8_t  rbuf[32] = {0};
    volatile uint8_t *flash = (volatile uint8_t *)0x90000000;

    printf("\r\n===== QSPI Test 2 =====\r\n");

    /* 1. 擦除 + 写入 */
    W25QXX_EraseSector(0x000000);
    W25QXX_WritePage(0x000000, wbuf, strlen((char *)wbuf));

    /* 2. 普通读验证 */
    W25QXX_Read(0x000000, rbuf, strlen((char *)wbuf));
    printf("Normal read: %s\r\n", rbuf);

    if (memcmp(wbuf, rbuf, strlen((char *)wbuf)) != 0) {
        printf("Write FAIL, stop.\r\n");
        return;
    }

    /* 3. 复位 QSPI 状态，手动配置内存映射 */
    /* 先禁用 QSPI */
    QUADSPI->CR &= ~QUADSPI_CR_EN;
    while (QUADSPI->CR & QUADSPI_CR_EN);

    /* 配置 DCR：FSIZE = 23 (8MB), CSHT = 3, CKMODE = 0 */
    QUADSPI->DCR = (23 << QUADSPI_DCR_FSIZE_Pos)
                 | (3  << QUADSPI_DCR_CSHT_Pos)
                 | (0  << QUADSPI_DCR_CKMODE_Pos);

    /* 配置 CCR：0xEB + 4线地址 + 4线数据 + 6 Dummy + FMODE=11 */
QUADSPI->CCR = (0xEB << QUADSPI_CCR_INSTRUCTION_Pos)
             | (1    << QUADSPI_CCR_IMODE_Pos)
             | (3    << QUADSPI_CCR_ADMODE_Pos)
             | (2    << QUADSPI_CCR_ADSIZE_Pos)   /* 24 位地址 */
             | (0    << QUADSPI_CCR_ABMODE_Pos)
             | (3    << QUADSPI_CCR_DMODE_Pos)
             | (6    << QUADSPI_CCR_DCYC_Pos)      /* 0xEB: M7-M0(2clk) + 4 dummy = 6 */
             | (3    << QUADSPI_CCR_FMODE_Pos);
		
    /* 配置 DLR = 0，ABR = 0 */
    QUADSPI->DLR = 0;
    QUADSPI->ABR = 0;

    /* 使能 QSPI */
    QUADSPI->CR |= QUADSPI_CR_EN;

    /* 诊断寄存器 */
    printf("QSPI CR  = 0x%08X\r\n", QUADSPI->CR);
    printf("QSPI DCR = 0x%08X\r\n", QUADSPI->DCR);
    printf("QSPI CCR = 0x%08X (FMODE bit27:26)\r\n", QUADSPI->CCR);

    /* 4. 清 D-Cache */
    SCB_InvalidateDCache_by_Addr((uint32_t *)0x90000000, 4096);

    /* 5. 读单字节 */
    printf("Before read flash[0]...\r\n");
    printf("flash[0] = 0x%02X (expect 0x4D 'M')\r\n", flash[0]);
    printf("flash[1] = 0x%02X (expect 0x61 'a')\r\n", flash[1]);
    printf("flash[2] = 0x%02X (expect 0x70 'p')\r\n", flash[2]);

    /* 6. 读字符串 */
    printf("Mapped read: %s\r\n", (char *)flash);

    /* 7. 对比 */
    if (memcmp((void *)flash, wbuf, strlen((char *)wbuf)) == 0) {
        printf("QSPI Test2 PASS\r\n");
    } else {
        printf("QSPI Test2 FAIL\r\n");
    }
}

void W25QXX_Test3(void)
{
    uint8_t buf[16] = {0};
    QSPI_CommandTypeDef cmd = {0};

    printf("\r\n===== QSPI Test 3 (Quad Read) =====\r\n");

    /* 先用 1 线写入数据 */
    W25QXX_EraseSector(0x000000);
    W25QXX_WritePage(0x000000, (uint8_t *)"Quad Mode OK", 12);

    /* 用 0xEB 四线读 */
    cmd.Instruction = 0xEB;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Address = 0x000000;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;      /* 4 线地址 */
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;   /* 先不用 */
    cmd.DataMode = QSPI_DATA_4_LINES;            /* 4 线数据 */
    cmd.NbData = 12;
    cmd.DummyCycles = 6;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("Quad cmd FAIL\r\n");
        return;
    }
    if (HAL_QSPI_Receive(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("Quad recv FAIL\r\n");
        return;
    }

    printf("Quad read: %s\r\n", buf);
}

void W25QXX_Test4(void)
{
    uint8_t buf[16] = {0};
    QSPI_CommandTypeDef cmd = {0};

    printf("\r\n===== QSPI Test 4 =====\r\n");

    /* 先用 1 线写入数据 */
    W25QXX_EraseSector(0x000000);
    W25QXX_WritePage(0x000000, (uint8_t *)"Mapped Test", 12);

    /* 用 0xEB 四线读，和内存映射一样的参数 */
    cmd.Instruction = 0xEB;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.Address = 0x000000;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.AlternateBytes = 0;
    cmd.AlternateBytesSize = 0;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.NbData = 12;
    cmd.DummyCycles = 6;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("Cmd FAIL\r\n");
        return;
    }
    if (HAL_QSPI_Receive(&hqspi, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        printf("Recv FAIL\r\n");
        return;
    }

    printf("Quad read: %s\r\n", buf);
}