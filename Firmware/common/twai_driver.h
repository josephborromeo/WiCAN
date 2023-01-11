#include "freertos/FreeRTOS.h"

#define TWAI_TX_Pin 12
#define TWAI_RX_Pin 13

#define RX_Timeout portMAX_DELAY

void initCAN(void);
void CAN_TX_Task(void*);
void CAN_RX_Task(void*);
