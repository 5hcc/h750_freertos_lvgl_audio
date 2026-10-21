#ifndef __KEY_H__
#define __KEY_H__

#include "main.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
	GPIO_PinState Current;
	GPIO_PinState Previous;
	
	GPIO_TypeDef *GPIO_Port;
	uint16_t GPIO_Pin;
	struct KeyInfo *next;
	void(*Key_callback)(void);
	
}KeyInfo;

extern KeyInfo hkey1;
extern KeyInfo hkey2;
extern KeyInfo hkey3;

void key_init(KeyInfo *Handel);
void key_scan(KeyInfo *Handel);

#endif