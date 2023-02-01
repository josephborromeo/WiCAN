#include "driver/twai.h"


typedef enum {
    CAN_FRAME,
    TEMP_DATA,
    LED_INFO,
} espnow_data_t;

// Custom packet datastructure
typedef struct{
    espnow_data_t data_type;
    twai_message_t data;
} wican_data_t;

