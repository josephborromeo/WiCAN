#include "temp_sensor.h"

#include "driver/temperature_sensor.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define ADC_TO_MV(adc_reading) (float)(adc_reading*1750/4095.0)
#define MV_TO_TEMP_C(temp_mv) (float)((temp_mv - 500)/10.0)
static const char *TAG = "TEMP";

adc_oneshot_unit_handle_t temp_adc_handle;
adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
};

//TODO: Split this task into external and internal temp tasks
//TODO: Clean up some init stuff and probably make an init function for external and internal temp sensors

void poll_board_temp(void*){
    // Setup External Temp Sesnor
    adc_oneshot_new_unit(&init_config1, &temp_adc_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_6,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(temp_adc_handle, Board_Temp_ADC_Channel, &config));
    
    // Setup Internal Temp sensor
    temperature_sensor_handle_t temp_handle = NULL;
    temperature_sensor_config_t temp_sensor = {
        .range_min = -10,
        .range_max = 80,
    };
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor, &temp_handle));
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));

    static int temp_mv; //adc1_get_raw(Board_Temp_ADC_Channel);
    static float tsens_out;
    while (1){
        // temp_mv = adc1_get_raw(Board_Temp_ADC_Channel);
        adc_oneshot_read(temp_adc_handle, Board_Temp_ADC_Channel, &temp_mv);
        // Read ESP temp
        ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));

        // ESP_LOGI(TAG, "Board Temp Voltage: %.2f mV", ADC_TO_MV(temp_mv));
        ESP_LOGI(TAG, "Board Temp: %.2f C \n Internal Temp: %.2f C", MV_TO_TEMP_C(ADC_TO_MV(temp_mv)), tsens_out);
        fprintf(stderr, "USB Serial print -> stderr\n");
        // ESP_LOGI(TAG, "ESP Internal Temp sensor: %.2f C", tsens_out);
        ESP_LOGI(TAG, "Unused Stack for Temp Task: %u", uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(pdMS_TO_TICKS(2000));    // Sleep for 2s
    }
    
}