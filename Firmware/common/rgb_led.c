#include "rgb_led.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


static const char *TAG = "LED";

ledc_channel_config_t ledc_channel[3] = {
        {
            .speed_mode     = LEDC_MODE,
            .channel        = LED_R_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = LED_R_Pin,
            .duty           = LEDC_DUTY, // Set duty to 100%
            .hpoint         = 0
        },
        {
            .speed_mode     = LEDC_MODE,
            .channel        = LED_G_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = LED_G_Pin,
            .duty           = LEDC_DUTY, // Set duty to 100%
            .hpoint         = 0
        },
        {
            .speed_mode     = LEDC_MODE,
            .channel        = LED_B_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = LED_B_Pin,
            .duty           = LEDC_DUTY, // Set duty to 100%
            .hpoint         = 0
        }
        
    };

void config_led(void){
    gpio_reset_pin(LED_R_Pin);
    gpio_reset_pin(LED_G_Pin);
    gpio_reset_pin(LED_B_Pin);

    /* Set the GPIOs as a push/pull output */
    gpio_set_direction(LED_R_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_G_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_B_Pin, GPIO_MODE_OUTPUT);

}


void set_led_color(uint8_t state){
    /*
    LED outputs are inverted - 0 = on, 1 = off
    */
    switch(state) {
        case LED_OFF:
            gpio_set_level(LED_G_Pin, 1);
            gpio_set_level(LED_B_Pin, 1);
            gpio_set_level(LED_R_Pin, 1);
        break;
        case LED_RED:
            gpio_set_level(LED_R_Pin, 0);
            gpio_set_level(LED_G_Pin, 1);
            gpio_set_level(LED_B_Pin, 1);
        break;
        case LED_GREEN:
            gpio_set_level(LED_R_Pin, 1);
            gpio_set_level(LED_G_Pin, 0);
            gpio_set_level(LED_B_Pin, 1);
        break;
        case LED_BLUE:
            gpio_set_level(LED_R_Pin, 1);
            gpio_set_level(LED_G_Pin, 1);
            gpio_set_level(LED_B_Pin, 0);
        break;
        case LED_WHITE:
            gpio_set_level(LED_R_Pin, 0);
            gpio_set_level(LED_G_Pin, 0);
            gpio_set_level(LED_B_Pin, 0);
        break;
        default:
            // code block
    }
}

void cycle_led(void*){
    /*
    Cycles LED between Off, Red, Green, Blue, White
    depending on the current state variable
    */

   static uint8_t curr_state = 0;

   while(1){
    set_led_color(curr_state);
    curr_state++;
    if (curr_state > LED_WHITE) curr_state=0;
    ESP_LOGI(TAG, "Current LED state: %u", curr_state);
    // ESP_LOGI(TAG, "Unused Stack for LED Task: %u", uxTaskGetStackHighWaterMark(NULL));
    
    vTaskDelay(pdMS_TO_TICKS(1000));    // Sleep for 1s

   }
}

void ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    for (int ch = 0; ch < NUM_COLORS; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }
}

void rainbow_cycle(void*){
    uint8_t step_size = 2;
    uint8_t max_brightness = 40; // Percent of max brightness
    uint8_t min_duty = LEDC_DUTY - (LEDC_DUTY*(max_brightness/100.0));
    ledc_init();

    while (1){
        for (int color=0; color < NUM_COLORS; color++){
            for (int intensity=LEDC_DUTY; intensity > (min_duty - step_size); intensity-=step_size){
                ledc_set_duty(ledc_channel[color].speed_mode, ledc_channel[color].channel, intensity);
                ledc_update_duty(ledc_channel[color].speed_mode, ledc_channel[color].channel);
                vTaskDelay(pdMS_TO_TICKS(30));
            }
        }
        for (int color=0; color < NUM_COLORS; color++){
            for (int intensity=min_duty; intensity < (LEDC_DUTY - step_size); intensity+=5){
                ledc_set_duty(ledc_channel[color].speed_mode, ledc_channel[color].channel, intensity);
                ledc_update_duty(ledc_channel[color].speed_mode, ledc_channel[color].channel);
                vTaskDelay(pdMS_TO_TICKS(30));
            }
        }
    }
}
