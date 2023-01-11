#include "twai_driver.h"

#include "driver/gpio.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


//Initialize configuration structures using macro initializers
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_Pin, TWAI_RX_Pin, TWAI_MODE_NORMAL);       // Maybe change mode to TWAI_MODE_NO_ACK
// Can change rx_queue_len in the g_config if we want a larger message queue

twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

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
    uint32_t rcv_counter = 0;
    static uint32_t diff = 0;
    uint32_t last_time = pdTICKS_TO_MS(xTaskGetTickCount());
    /*
    Simple Receive task to get can MSG and forward it to the logging and transmit tasks
    If performance is an issue then look into CONFIG_TWAI_ISR_IN_IRAM to use the TWAI ISR - must setup in menuconfig as well
    */
    printf("Started CAN RX TASK\n");

    while (1){
        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(RX_Timeout)) == ESP_OK) {
            // printf("Message received\n");
            printf("ID: %lu\tExt ID: %i\tDLC:%u\t", message.identifier, message.extd, message.data_length_code);
            for (int i = 0; i < message.data_length_code; i++) {
                printf("%d ", message.data[i]);
            }
            printf("\n");
            rcv_counter++;
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
    // Take get input from queue and then send it out
}