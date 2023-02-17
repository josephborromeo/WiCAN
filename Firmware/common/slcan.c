#include "slcan.h"

const char* slcan_format(twai_message_t message){
    char output_string[19]; // Max Length of SLCAN Message
    // Start of Frame
    strcat(output_string, SOF);

    // Timestamp

    // DLC

    // Arbitration ID

    // Payload (Data)

    // End of Frame
    strcat(output_string, EOF);

    return output_string;
}