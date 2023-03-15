#include "driver/twai.h"

// maximum rx buffer len: extended CAN frame with timestamp 
#define SLCAN_MTU 30 // (sizeof("T1111222281122334455667788EA5F\r")+1)

#define SLCAN_STD_ID_LEN 3      // STD ID = 11 bits = 2.75 nibbles
#define SLCAN_EXT_ID_LEN 8      // EXT ID = 29 bits = 7.25 nibbles


uint8_t slcan_format(uint8_t* msg_buffer, twai_message_t message);
int8_t slcan_parse_str(uint8_t *buf, uint8_t len, twai_message_t* message);