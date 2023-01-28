#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

/*
            FIXME
fix the CMakeLists.txt file to add EXTRA_COMPONENT_DIRS (?maybe?) so that we dont have to do relative directories when importing common files 

*/

// Common Includes
#include "../common/rgb_led.h"
#include "../common/temp_sensor.h"
#include "../common/twai_driver.h"
#include "../common/wifi.h"

//RX Specific Includes

// Board Identifier:
#define WICAN_RX



static const char *TAG = "MAIN";

#define func_btn_Pin 0
#define ESP_INTR_FLAG_DEFAULT 0
