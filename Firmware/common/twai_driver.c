#include "twai_driver.h"

#include "driver/gpio.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "wifi.h"
// #include "data_types.h"
#include  "../WiCAN_TX/main/sdmmc_driver.h"     // FIXME


//Initialize configuration structures using macro initializers
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_Pin, TWAI_RX_Pin, TWAI_MODE_NORMAL);
// Can change rx_queue_len in the g_config if we want a larger message queue
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

// Create queue to process incoming CAN frames
QueueHandle_t rx_can_queue, tx_can_queue, sd_can_queue;

void initCAN(void){
    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        printf("CAN Driver installed\n");
    } else {
        printf("Failed to install CAN driver\n");
        return;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        printf("CAN Driver started\n");
    } else {
        printf("Failed to start CAN driver\n");
        return;
    }

}

void CAN_RX_Task(void*){
    static uint32_t rcv_counter = 0;
    static uint32_t diff = 0;
    uint32_t last_time = pdTICKS_TO_MS(xTaskGetTickCount());
    /*
    Simple Receive task to get can MSG and forward it to the logging and transmit tasks
    If performance is an issue then look into CONFIG_TWAI_ISR_IN_IRAM to use the TWAI ISR - must setup in menuconfig as well
    */
    printf("Started CAN RX TASK\n");

    while (1){
        twai_message_t message;
        CAN_frame_t can_msg;
        if (twai_receive(&message, pdMS_TO_TICKS(RX_Timeout)) == ESP_OK) {
            // See if we want to process std frames - always process extended frames
            if ((PROCESS_STD_FRAMES && !message.extd) || message.extd){
                if (xQueueSend(rx_can_queue, &message, (TickType_t)100) != pdTRUE){   // Change timeout to 0 to check if queue is full
                    printf("Wifi Queue Full\n");
                }  // Try changing to 0

                can_msg.extd = message.extd;
                can_msg.identifier = message.identifier;
                can_msg.data_length_code = message.data_length_code;
                memcpy(can_msg.data, message.data, can_msg.data_length_code);
                if (xQueueSend(sd_can_queue, &message, (TickType_t)100) != pdTRUE){
                    printf("SD Queue Full\n");
                }  // Try changing to 0
                rcv_counter++;
            }
            
        } 
        else {
            printf("Failed to receive message\n");
            printf("Time: %lu ms\n", pdTICKS_TO_MS(xTaskGetTickCount()));
        }

        diff = pdTICKS_TO_MS(xTaskGetTickCount()) - last_time;
        if (diff >= 1000){  // Print message every second
            printf("%.2f msgs / second\n", (((float)rcv_counter/diff)*1000.0));
            rcv_counter = 0;
            last_time = pdTICKS_TO_MS(xTaskGetTickCount());
        }
    }
}

void CAN_TX_Task(void*){
    tx_can_queue = xQueueCreate(PROCESS_QUEUE_SIZE, sizeof(wican_data_t));
    while (1){
        wican_data_t data_packet = {0};
        twai_message_t message;
        if(xQueueReceive(tx_can_queue, &(data_packet), (TickType_t)portMAX_DELAY)) {   
            message.extd = data_packet.frames[0].extd;
            message.identifier = data_packet.frames[0].identifier;
            message.data_length_code = data_packet.frames[0].data_length_code;
            memcpy(message.data, data_packet.frames[0].data, data_packet.frames[0].data_length_code); 
            if (twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK) {
                printf("Sent Message onto CAN Bus\n");
            }
        }
    }
}

void process_CAN_frame(void*) {
    rx_can_queue = xQueueCreate(PROCESS_QUEUE_SIZE, sizeof(twai_message_t));
    twai_message_t message;
    static uint32_t rcv_counter = 0;
    uint32_t last_time = pdTICKS_TO_MS(xTaskGetTickCount());
    uint32_t last_sent_time = pdTICKS_TO_MS(xTaskGetTickCount());
    static uint32_t diff = 0;
    
    while (1) {
        // Check if enough messages to fill msg array, or if > 100 ms has surpassed
        if (uxQueueMessagesWaiting(rx_can_queue) >= MAX_FRAMES ||((pdTICKS_TO_MS(xTaskGetTickCount()) - last_sent_time >= MAX_SEND_DELAY_MS) && uxQueueMessagesWaiting(rx_can_queue) > 0)){
            wican_data_t data_packet = {0};
            static uint8_t i=0;
            uint8_t min_num = MIN(uxQueueMessagesWaiting(rx_can_queue), MAX_FRAMES);

            for (i=0; i < min_num; i++){
                if(xQueueReceive(rx_can_queue, &(message), (TickType_t)portMAX_DELAY)) {
                    data_packet.frames[i].extd = message.extd;
                    data_packet.frames[i].identifier = message.identifier;
                    data_packet.frames[i].data_length_code = message.data_length_code;
                    data_packet.num_frames++;
                }
            }
            // printf("num_frames: %u frames\n", data_packet.num_frames);
            send_to_all(data_packet);
            rcv_counter += i;

            diff = pdTICKS_TO_MS(xTaskGetTickCount()) - last_time;
            if (diff >= 1000){  // Print message every second
                printf("%.2f Wifi msgs / second\n", (((float)rcv_counter/diff)*1000.0));
                rcv_counter = 0;
                last_time = pdTICKS_TO_MS(xTaskGetTickCount());
            }

            last_sent_time = pdTICKS_TO_MS(xTaskGetTickCount());
        }
        // else{
        //     vTaskDelay(1);
        // }
    }
}

void sd_write_task(void*) {
    sd_can_queue = xQueueCreate(PROCESS_QUEUE_SIZE, sizeof(twai_message_t));

    twai_message_t message;
    static uint32_t rcv_counter = 0;
    uint32_t last_time = pdTICKS_TO_MS(xTaskGetTickCount());
    static uint32_t diff = 0;
    while (1) {
        if( xQueueReceive(sd_can_queue, &(message), (TickType_t)portMAX_DELAY)) {
            write_to_sd(message);
            rcv_counter++;

            diff = pdTICKS_TO_MS(xTaskGetTickCount()) - last_time;
            if (diff >= 1000){  // Print message every second
                printf("%.2f sd msgs / second\n", (((float)rcv_counter/diff)*1000.0));
                rcv_counter = 0;
                last_time = pdTICKS_TO_MS(xTaskGetTickCount());
            }
        }
    }
}

