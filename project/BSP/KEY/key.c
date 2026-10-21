#include "key.h"

static void key1callback();
static void key2callback();	
static void key3callback();	
 
	KeyInfo hkey3 = {
		.GPIO_Port 	= 	GPIOH,
		.GPIO_Pin 	= 	GPIO_PIN_3,
		.next = NULL,
		.Key_callback = key3callback 
	};

	KeyInfo hkey2 = {
		.GPIO_Port 	= 	GPIOC,
		.GPIO_Pin 	= 	GPIO_PIN_13,
		.next = &hkey3,
		.Key_callback = key2callback 
	};
	
	KeyInfo hkey1 = {
		.GPIO_Port 	= 	GPIOH,
		.GPIO_Pin 	= 	GPIO_PIN_2,
		.next = &hkey2,
		.Key_callback = key1callback 
	};

void key_init(KeyInfo *Handel)
{
	Handel->Current = 1;
	Handel->Previous = 1;
}

void key_scan(KeyInfo *Handel)
{
    Handel->Current = HAL_GPIO_ReadPin(Handel->GPIO_Port, Handel->GPIO_Pin);

    if (Handel->Current != Handel->Previous && Handel->Current == GPIO_PIN_SET) 
	{
        vTaskDelay(pdMS_TO_TICKS(20));

        if (HAL_GPIO_ReadPin(Handel->GPIO_Port, Handel->GPIO_Pin) == GPIO_PIN_SET)
            Handel->Key_callback();

    }
	
    Handel->Previous = Handel->Current;
}

void key1callback()
{
	extern UART_HandleTypeDef huart3;
	HAL_UART_Transmit(&huart3, (uint8_t *)"key1\r\n", 8, 100);
}

void key2callback()
{
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	
}

void key3callback()
{
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
	
}