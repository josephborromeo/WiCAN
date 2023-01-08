#include <stdint.h>

#define LED_R_Pin 4
#define LED_R_CHANNEL LEDC_CHANNEL_0
#define LED_G_Pin 5
#define LED_G_CHANNEL LEDC_CHANNEL_1
#define LED_B_Pin 6
#define LED_B_CHANNEL LEDC_CHANNEL_2

#define NUM_COLORS 3

// LEDC Defines
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT // Set duty resolution to 8 bits
#define LEDC_DUTY               (255) // Set duty to 100%
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

enum led_states{
    LED_OFF,    // 0
    LED_RED,    // 1
    LED_GREEN,  // 2
    LED_BLUE,   // 3
    LED_WHITE,  // 4
};


void config_led(void);
void set_led_color(uint8_t state);
void cycle_led(void*);
void ledc_init(void);
void rainbow_cycle(void*);
