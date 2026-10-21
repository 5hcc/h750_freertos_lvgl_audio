#include "es8388.h" 
#include "PCF8574/pcf8574.h" 

uint8_t es8388_init()
{
	i2c_init();
	
	pcf8574_write_bit(4, 0);
	
	  i2c_wr_reg(0, 0x80);      /* 软复位ES8388 */
    i2c_wr_reg(0, 0x00);
    delay_ms(100);                  /* 等待复位 */

    i2c_wr_reg(0x01, 0x58);
    i2c_wr_reg(0x01, 0x50);
    i2c_wr_reg(0x02, 0xF3);
    i2c_wr_reg(0x02, 0xF0);

    i2c_wr_reg(0x03, 0x09);   /* 麦克风偏置电源关闭 */
    i2c_wr_reg(0x00, 0x06);   /* 使能参考 500K驱动使能 */
    i2c_wr_reg(0x04, 0x00);   /* DAC电源管理，不打开任何通道 */
    i2c_wr_reg(0x08, 0x00);   /* MCLK不分频 */
    i2c_wr_reg(0x2B, 0x80);   /* DAC控制	DACLRC与ADCLRC相同 */

    i2c_wr_reg(0x09, 0x88);   /* ADC L/R PGA增益配置为+24dB */
    i2c_wr_reg(0x0C, 0x4C);   /* ADC 数据选择为left data = left ADC, right data = left ADC  音频数据为16bit */
    i2c_wr_reg(0x0D, 0x02);   /* ADC配置 MCLK/采样率=256 */
    i2c_wr_reg(0x10, 0x00);   /* ADC数字音量控制将信号衰减 L  设置为最小！！！ */
    i2c_wr_reg(0x11, 0x00);   /* ADC数字音量控制将信号衰减 R  设置为最小！！！ */

    i2c_wr_reg(0x17, 0x18);   /* DAC 音频数据为16bit */
    i2c_wr_reg(0x18, 0x02);   /* DAC 配置 MCLK/采样率=256 */
    i2c_wr_reg(0x1A, 0x00);   /* DAC数字音量控制将信号衰减 L  设置为最小！！！ */
    i2c_wr_reg(0x1B, 0x00);   /* DAC数字音量控制将信号衰减 R  设置为最小！！！ */
    i2c_wr_reg(0x27, 0xB8);   /* L混频器 */
    i2c_wr_reg(0x2A, 0xB8);   /* R混频器 */
    delay_ms(100);
    
    return 0;
}

/**
 * @brief       设置ES8388工作模式
 * @param       fmt : 工作模式
 *    @arg      0, 飞利浦标准I2S;
 *    @arg      1, MSB(左对齐);
 *    @arg      2, LSB(右对齐);
 *    @arg      3, PCM/DSP
 * @param       len : 数据长度
 *    @arg      0, 24bit
 *    @arg      1, 20bit 
 *    @arg      2, 18bit 
 *    @arg      3, 16bit 
 *    @arg      4, 32bit 
 * @retval      无
 */
void es8388_sai_cfg(uint8_t fmt, uint8_t len)
{
    fmt &= 0x03;
    len &= 0x07;                                    /* 限定范围 */
    i2c_wr_reg(23, (fmt << 1) | (len << 3));  /* R23,ES8388工作模式设置 */
}

/**
 * @brief       设置耳机音量
 * @param       voluem : 音量大小(0 ~ 33)
 * @retval      无
 */
void es8388_hpvol_set(uint8_t volume)
{
    if (volume > 33)
    {
        volume = 33;
    }
    
    i2c_wr_reg(0x2E, volume);
    i2c_wr_reg(0x2F, volume);
}

/**
 * @brief       设置喇叭音量
 * @param       volume : 音量大小(0 ~ 33)
 * @retval      无
 */
void es8388_spkvol_set(uint8_t volume)
{
    if (volume > 33)
    {
        volume = 33;
    }
    
    i2c_wr_reg(0x30, volume);
    i2c_wr_reg(0x31, volume);
}

/**
 * @brief       设置3D环绕声
 * @param       depth : 0 ~ 7(3D强度,0关闭,7最强)
 * @retval      无
 */
void es8388_3d_set(uint8_t depth)
{ 
    depth &= 0x7;                           /* 限定范围 */
    i2c_wr_reg(0x1D, depth << 2);     /* R7,3D环绕设置 */
}

/**
 * @brief       ES8388 DAC/ADC配置
 * @param       dacen : dac使能(1)/关闭(0)
 * @param       adcen : adc使能(1)/关闭(0)
 * @retval      无
 */
void es8388_adda_cfg(uint8_t dacen, uint8_t adcen)
{
    uint8_t tempreg = 0;
    
    tempreg |= ((!dacen) << 0);
    tempreg |= ((!adcen) << 1);
    tempreg |= ((!dacen) << 2);
    tempreg |= ((!adcen) << 3);
    i2c_wr_reg(0x02, tempreg);
}

/**
 * @brief       ES8388 DAC输出通道配置
 * @param       o1en : 通道1使能(1)/禁止(0)
 * @param       o2en : 通道2使能(1)/禁止(0)
 * @retval      无
 */
void es8388_output_cfg(uint8_t o1en, uint8_t o2en)
{
    uint8_t tempreg = 0;
    tempreg |= o1en * (3 << 4);
    tempreg |= o2en * (3 << 2);
    i2c_wr_reg(0x04, tempreg);
}

/**
 * @brief       ES8388 MIC增益设置(MIC PGA增益)
 * @param       gain : 0~8, 对应0~24dB  3dB/Step
 * @retval      无
 */
void es8388_mic_gain(uint8_t gain)
{
    gain &= 0x0F;
    gain |= gain << 4;
    i2c_wr_reg(0x09, gain);       /* R9,左右通道PGA增益设置 */
}

/**
 * @brief       ES8388 ALC设置
 * @param       sel
 *   @arg       0,关闭ALC
 *   @arg       1,右通道ALC
 *   @arg       2,左通道ALC
 *   @arg       3,立体声ALC
 * @param       maxgain : 0~7,对应-6.5~+35.5dB
 * @param       mingain : 0~7,对应-12~+30dB 6dB/STEP
 * @retval      无
 */
void es8388_alc_ctrl(uint8_t sel, uint8_t maxgain, uint8_t mingain)
{
    uint8_t tempreg = 0;
    
    tempreg = sel << 6;
    tempreg |= (maxgain & 0x07) << 3;
    tempreg |= mingain & 0x07;
    i2c_wr_reg(0x12, tempreg);     /* R18,ALC设置 */
}

/**
 * @brief       ES8388 ADC输出通道配置
 * @param       in : 输入通道
 *    @arg      0, 通道1输入
 *    @arg      1, 通道2输入
 * @retval      无
 */
void es8388_input_cfg(uint8_t in)
{
    i2c_wr_reg(0x0A, (5 * in) << 4);   /* ADC1 输入通道选择L/R  INPUT1 */
}


