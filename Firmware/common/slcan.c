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

// Parse an incoming slcan command from the USB CDC port
uint8_t slcan_parse_str(uint8_t *buf, uint8_t len, twai_message_t* message){

	// Default to extended ID unless otherwise specified
    message->extd = 1;
    message->identifier = 0;


    // Convert from ASCII (2nd character to end)
    for (uint8_t i = 1; i < len; i++)
    {
        // Lowercase letters
        if(buf[i] >= 'a')
            buf[i] = buf[i] - 'a' + 10;
        // Uppercase letters
        else if(buf[i] >= 'A')
            buf[i] = buf[i] - 'A' + 10;
        // Numbers
        else
            buf[i] = buf[i] - '0';
    }


    // Process command
    switch(buf[0])
    {
		case 'O':
			// Open channel command
			// can_enable();

            // TODO: Implement

			return 0;

		case 'C':
			// Close channel command
			// can_disable();

            // TODO: Implement
			return 0;

		case 'S':
			// Set bitrate command
            // FIXME: Implement
			// // Check for valid bitrate
			// if(buf[1] >= CAN_BITRATE_INVALID)
			// {
			// 	return 0;
			// }

			// can_set_bitrate(buf[1]);
			// return 0;


        // TODO: Implement
		// case 'm':
		// case 'M':
		// 	// Set mode command
		// 	if (buf[1] == 1)
		// 	{
		// 		// Mode 1: silent
		// 		can_set_silent(1);
		// 	} else {
		// 		// Default to normal mode
		// 		can_set_silent(0);
		// 	}
		// 	return 0;

		// case 'a':
		// case 'A':
		// 	// Set autoretry command
		// 	if (buf[1] == 1)
		// 	{
		// 		// Mode 1: autoretry enabled (default)
		// 		can_set_autoretransmit(1);
		// 	} else {
		// 		// Mode 0: autoretry disabled
		// 		can_set_autoretransmit(0);
		// 	}
		// 	return 0;

		// case 'V':
		// {
		// 	// Report firmware version and remote
		// 	char* fw_id = GIT_VERSION " " GIT_REMOTE "\r";
		// 	CDC_Transmit_FS((uint8_t*)fw_id, strlen(fw_id));
		// 	return 0;
		// }

	    // // Nonstandard!
		// case 'E':
		// {
	    //     // Report error register
		// 	char errstr[64] = {0};
		// 	snprintf_(errstr, 64, "CANable Error Register: %X", (unsigned int)error_reg());
		// 	CDC_Transmit_FS((uint8_t*)errstr, strlen(errstr));
	    //     return 0;
		// }

        case 't':
            message->extd = 0;
            break;
		case 'T':
	    	message->extd = 1;
            break;

		// case 't':
		// 	// Transmit data frame command
		// 	frame_header.RTR = CAN_RTR_DATA;
		// 	break;


        // TODO: Implement remote frames
		// case 'R':
	    // 	frame_header.IDE = CAN_ID_EXT;
		// case 'r':
		// 	// Transmit remote frame command
		// 	frame_header.RTR = CAN_RTR_REMOTE;
		// 	break;

    	default:
    		// Error, unknown command
    		return 0;
    }


    // Save CAN ID depending on ID type
    uint8_t msg_position = 1;
    if (message->extd == 1) {
        while (msg_position <= SLCAN_EXT_ID_LEN) {
        	message->identifier *= 16;
        	message->identifier += buf[msg_position++];
        }
    }
    /*   TODO: Implement Stanard IDs    */
    // else {
    //     while (msg_position <= SLCAN_STD_ID_LEN) {
    //     	frame_header.StdId *= 16;
    //     	frame_header.StdId += buf[msg_position++];
    //     }
    // }


    // Attempt to parse DLC and check sanity
    message->data_length_code = buf[msg_position++];
    if (message->data_length_code > 8) {
        return 0;
    }

    // Copy frame data to buffer
    for (uint8_t i = 0; i < message->data_length_code; i++) {
        message->data[i] = (buf[msg_position] << 4) + buf[msg_position+1];
        msg_position += 2;
    }

    // Transmit the message

    // Add twai_message_t pointer to params and just update that - do not want to send from here
    // can_tx(&frame_header, frame_data);

    return 0;
}
