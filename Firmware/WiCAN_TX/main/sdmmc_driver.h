#include <stdio.h>
#include "driver/sdmmc_host.h"
#include "driver/twai.h"


/*
 *  CAN Logs Format: Follows candump general output + timestamp at the front 
 *          (timestamp)    interface    MSG_ID  [DLC]  data
 * EX:      (0946685107.875001)  can1  18C00401   [8]  0F BC 32 C1 32 B8 32 00
*/

// Use defines for pin numbers probably
#define MOUNT_POINT "/sd"

#define LOG_NAME "CANlog"

extern FILE * fp;

void init_sd_card(void);
void create_log_file(void);

void write_to_sd(twai_message_t message);
void log_CAN_messages(void *);