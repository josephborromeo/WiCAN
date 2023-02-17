#include "slcan.h"

#include "driver/twai.h"

// Convert Incoming CAN message into SLCAN formatted "string"
const char* slcan_format(twai_message_t message){
    uint8_t msg_buffer[SLCAN_MTU]; // Max Length of SLCAN Message
    
    // Fill Buffer with null chars
    for (uint8_t j=0; j < SLCAN_MTU; j++)
    {
        msg_buffer[j] = '\0';
    }

    return output_string;
}