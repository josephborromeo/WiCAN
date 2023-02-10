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

// Common Includes
#include "../common/rgb_led.h"
#include "../common/temp_sensor.h"
#include "../common/wifi.h"

//RX Specific Includes
#include "usb_driver.h"

static const char *TAG = "MAIN";

#define func_btn_Pin 0
#define ESP_INTR_FLAG_DEFAULT 0


void app_main(void)
{   
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    config_led();
    ESP_LOGI(TAG, "Configured GPIO");
    

    // usb_init();

    printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());

    xTaskCreate(rainbow_cycle, "LED_Task", 2500, NULL, 5, NULL);
    xTaskCreate(&poll_board_temp, "Temp_Task", 2500, NULL, 5, NULL);
    xTaskCreate(&parse_incoming, "CAN_Parse_Task", 5000, NULL, 8, NULL);
    
    wifi_init();    // Start Wifi Last to avoid boot issues
}
