#include "qspi.h"
#include "DELAY\delay.h"

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
    QSPI_CommandTypeDef sCommand = {0};
    QSPI_MemoryMappedTypeDef sMemMappedCfg = {0};

    sCommand.Instruction        = 0xEB;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_4_LINES;
    sCommand.AddressSize        = QSPI_ADDRESS_24_BITS;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.AlternateBytes     = 0x00;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 6;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    sMemMappedCfg.TimeOutPeriod     = 0;

    HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg);
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