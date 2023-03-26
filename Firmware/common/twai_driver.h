#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define TWAI_TX_Pin 12
#define TWAI_RX_Pin 13

#define RX_Timeout portMAX_DELAY

// Set max number of can frames to store in process queue
#define PROCESS_QUEUE_SIZE 25

#define PROCESS_STD_FRAMES false

extern QueueHandle_t tx_can_queue;


void initCAN(void);
void CAN_TX_Task(void*);
void CAN_RX_Task(void*);

void process_CAN_frame(void*);