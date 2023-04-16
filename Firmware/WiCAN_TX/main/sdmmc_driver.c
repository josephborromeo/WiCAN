#include "sdmmc_driver.h"

#include <string.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "dirent.h"
#include "freertos/FreeRTOS.h"
#include "errno.h"

#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "SD";

FILE * fp;
bool s_fat_mounted;
sdmmc_card_t *card;

void init_sd_card(void){
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // Set SDMMC speed to 40MHz
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();

    slot.clk = 37;
    slot.cmd = 38;
    slot.d0 = 36;
    slot.d1 = 35;
    slot.d2 = 40;
    slot.d3 = 39;
    slot.width = 4;
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 64 * 512        // 512 is sector size. 128x sector size is max
    };


    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI(TAG, "Mounting filesystem");
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "Failed to mount card");
        s_fat_mounted = false;
        // TODO: Set LED to RED or something to show error

    } else{
        ESP_LOGI(TAG, "SD Card mounted successfully!");
        s_fat_mounted = true;
        sdmmc_card_print_info(stdout, card);
        create_log_file();
    }   

}

void create_log_file(void){
    DIR *dir = opendir(MOUNT_POINT);
    uint8_t name_len = 32;
    char log_name[name_len];
    uint16_t log_num = 0;
    uint16_t temp_num = 0;

    struct dirent *dp;

    while ((dp = readdir(dir)) != NULL) {
        printf("File: %s\n", dp->d_name);
        if (dp->d_type == DT_REG){
            if (sscanf(dp->d_name, "%*[^0123456789]%hu", &temp_num)){
                if (temp_num > log_num) log_num = temp_num;
                // printf("Log Num: %u\n", log_num);
            }
        }
    }
    closedir(dir);
    // printf("DIR CLOSED\n");

    log_num++;
    snprintf(log_name, name_len, "%s/%s%u.log", MOUNT_POINT, LOG_NAME, log_num);
    printf("New Log File: %s\n", log_name);

    
    fp = fopen(log_name, "w");
    setvbuf(fp, NULL, _IOFBF, 64 * 512);   // Set Buffer so it doesn't constantly write 
    if (fp == NULL){
        printf("Failed to Open File\n");
        printf( "Error code opening file: %d\n", errno );
        printf( "Error opening file: %s\n", strerror( errno ) );
        // TODO: Set LED to RED or something to show error
    }else{
        printf("Log file created successfully\n");
    }
    
}

/*
    Writes line to SD card `.log` file using the same format as candump on linux to make it compatible
    with our CANlogGUI software

    Example formatted line in .log file:
     (0946685107.875001)  can1  18C00401   [8]  0F BC 32 C1 32 B8 32 00
     (timestamp_seconds)  bus  messageID   [DLC]  D7 D6 D5 D4 D3 D2 D1 D0
*/
void write_to_sd(twai_message_t message){

    fprintf(fp, " (%.4f)  %s  %lX   [%u] ", pdTICKS_TO_MS(xTaskGetTickCount())/1000.0, "can1", message.identifier, message.data_length_code);
    for (int i = 0; i < message.data_length_code; i++) {
        fprintf(fp, " %02X", message.data[i]);
    }

    fprintf(fp, "\n");
    fsync(fileno(fp));
}


/*
SD Card sector funcs - used for USB MSC
*/

uint32_t storage_get_sector_count(void) {
    assert(card);

    return (uint32_t)card->csd.capacity;
}

uint32_t storage_get_sector_size(void) {
    assert(card);

    return (uint32_t)card->csd.sector_size;
}

esp_err_t storage_read_sector(uint32_t lba, uint32_t offset, size_t size, void *dest) {
    assert(card);
    return sdmmc_read_sectors(card, dest, lba, size / storage_get_sector_size());
}

esp_err_t storage_write_sector(uint32_t lba, uint32_t offset, size_t size, const void *src) {
    assert(card);

    if (s_fat_mounted) {
        ESP_LOGE(TAG, "can't write, FAT mounted");
        return ESP_ERR_INVALID_STATE;
    }
    size_t addr = lba * storage_get_sector_size() + offset; // Address of the data to be read, relative to the beginning of the partition.
    size_t sector_size = card->csd.sector_size;
    if (addr % sector_size != 0 || size % sector_size != 0) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_RETURN_ON_ERROR(sdmmc_erase_sectors(card, lba, size / storage_get_sector_size(), SDMMC_ERASE_ARG),
                        TAG, "Failed to erase");

    return sdmmc_write_sectors(card, src, lba, size / storage_get_sector_size());
}
