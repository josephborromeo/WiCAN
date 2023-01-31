#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"

/*
            FIXME
fix the CMakeLists.txt file to add EXTRA_COMPONENT_DIRS (?maybe?) so that we dont have to do relative directories when importing common files 

*/

// TODO: Add include guards to all header files
//  - https://stackoverflow.com/questions/1653958/why-are-ifndef-and-define-used-in-c-header-files

// Common Includes
#include "../common/rgb_led.h"
#include "../common/temp_sensor.h"
#include "../common/twai_driver.h"
#include "../common/wifi.h"

//TX Specific Includes
#include "sdmmc_driver.h"


static const char *TAG = "MAIN";

#define func_btn_Pin 0
#define ESP_INTR_FLAG_DEFAULT 0


// GPIO ISR and handler
static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
            printf("Time: %lu ms\n", pdTICKS_TO_MS(xTaskGetTickCount()));
            
        }
    }
}

static void config_func_button(void){
    gpio_set_direction(func_btn_Pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(func_btn_Pin, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(func_btn_Pin, GPIO_INTR_NEGEDGE);
}


void app_main(void)
{   
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );


    config_led();
    config_func_button();
    ESP_LOGI(TAG, "Configured GPIO");
    init_sd_card();      // Fix this to not crash if no SD card is in/ not formatted
    initCAN();
    
    wifi_init();
    
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    

    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 3000, NULL, 2, NULL);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(func_btn_Pin, gpio_isr_handler, (void*) func_btn_Pin);

    printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());

    // xTaskCreate(&cycle_led, "LED_Task", 2050, NULL, 5, NULL);
    xTaskCreate(rainbow_cycle, "LED_Task", 2500, NULL, 5, NULL);
    xTaskCreate(&poll_board_temp, "Temp_Task", 2500, NULL, 5, NULL);
    xTaskCreate(&CAN_RX_Task, "CAN_RX_Task", 6000, NULL, 2, NULL);
    xTaskCreate(&send_data_task, "ESP_NOW_TX_Task", 6000, NULL, 2, NULL);
}
