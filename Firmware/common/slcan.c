#include "slcan.h"


// Convert Incoming CAN message into SLCAN formatted "string"
// Currently only supports extended IDs - TODO: Support std ids
/*
 *  Format: TIIIIIIIILDD... - Transmit data frame (Extended ID) [ID, length, data]
 *  Each ascii char is a nibble
 * 
 *  Specification from can232_v3.pdf
*/
uint8_t slcan_format(uint8_t* msg_buffer, twai_message_t message){
    uint8_t msg_position = 0;

    // Fill Buffer with null chars
    for (uint8_t i=0; i < SLCAN_MTU; i++)
    {
        msg_buffer[i] = '\0';
    }


    msg_buffer[msg_position] = 'T';     // Only for Extended Frame
    msg_position++;

    uint32_t can_id = message.identifier;

    // Add Identifier
    for(uint8_t i = SLCAN_EXT_ID_LEN; i > 0; i--)
    {
        // Add nybble to buffer
        msg_buffer[i] = (can_id & 0xF);    // Check if the + msg_pos is correct
        can_id = can_id >> 4;
        msg_position++;
    }

    uint8_t dlc = message.data_length_code;
    // Add DLC
    msg_buffer[msg_position++] = dlc;

    // Add data bytes
    for (uint8_t j = 0; j < dlc; j++)
    {
        msg_buffer[msg_position++] = (message.data[j] >> 4);
        msg_buffer[msg_position++] = (message.data[j] & 0x0F);
    }

    // Convert to ASCII (2nd character to end)
    for (uint8_t j = 1; j < msg_position; j++)
    {
        if (msg_buffer[j] < 0xA) {
            msg_buffer[j] += 0x30;
        } else {
            msg_buffer[j] += 0x37;
        }
    }

    // Add CR (slcan EOL)
    msg_buffer[msg_position++] = '\r';

    return msg_position;
}