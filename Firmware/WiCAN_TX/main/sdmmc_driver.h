#pragma once

#ifndef SDMMC_DRIVER_H
#define SDMMC_DRIVER_H

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
#define LOG_NAME "CANlog_"

// sdmmc_host_t host;  // Might have to set these to defaults again
// sdmmc_slot_config_t slot; // Might have to set these to defaults again

extern FILE * fp;


void init_sd_card(void);
void create_log_file(void);

void write_to_sd(twai_message_t message);
void log_CAN_messages(void *);

uint32_t storage_get_sector_count(void);
uint32_t storage_get_sector_size(void);
esp_err_t storage_read_sector(uint32_t lba, uint32_t offset, size_t size, void *dest);
esp_err_t storage_write_sector(uint32_t lba, uint32_t offset, size_t size, const void *src);

#endif