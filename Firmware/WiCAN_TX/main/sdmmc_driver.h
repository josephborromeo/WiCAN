#include "driver/sdmmc_host.h"

/*
 *  CAN Logs Format: Follows candump general output + timestamp at the front 
 *          (timestamp)    interface    MSG_ID  [DLC]  data
 * EX:      (0946685107.875001)  can1  18C00401   [8]  0F BC 32 C1 32 B8 32 00
*/

// Use defines for pin numbers probably

#define MOUNT_POINT "/sdcard"

#define LOG_NAME "CANlog"

void init_sd_card(void);
void print_files(void);

void log_CAN_messages(void *);