#include "retarget.h"

extern UART_HandleTypeDef huart3;

int fputc(int ch, FILE *f)
{
    
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
