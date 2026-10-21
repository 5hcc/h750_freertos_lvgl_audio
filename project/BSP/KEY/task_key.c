#include "task_key.h"


void vKey1task(void *pvParameters)
{
    KeyInfo *phead = (KeyInfo *)pvParameters;
    KeyInfo *pcurr = phead;

    for (KeyInfo *p = phead; p != NULL; p = p->next)
        key_init(p);

    for (;;) {
        pcurr = phead;
        while (pcurr != NULL) {
            key_scan(pcurr); // 扫描当前按键
            pcurr = pcurr->next;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
