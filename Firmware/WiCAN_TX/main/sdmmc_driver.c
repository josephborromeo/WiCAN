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

#include "esp_log.h"

static const char *TAG = "SD";

void init_sd_card(void){
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
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
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;

    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI(TAG, "Mounting filesystem");
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "Failed to mount card");

    } else{
        ESP_LOGI(TAG, "SD Card mounted successfully!");
        sdmmc_card_print_info(stdout, card);
        print_files();
    }   

}

void print_files(void){
    DIR *dir = opendir(MOUNT_POINT);
    char log_name[32];
    uint16_t log_num = 0;
    uint16_t temp_num = 0;

    struct dirent *dp;

    while ((dp = readdir(dir)) != NULL) {
        printf("File: %s\n", dp->d_name);
        if (dp->d_type == DT_REG){
            if (sscanf(dp->d_name, "%*[^0123456789]%hu", &temp_num)){
                if (temp_num > log_num) log_num = temp_num;
                printf("Log Num: %u\n", log_num);
            }
        }
    }
    closedir(dir);
    printf("DIR CLOSED\n");

    log_num++;
    sprintf(log_name, "%s/%s_%u.log", MOUNT_POINT, LOG_NAME, log_num);
    printf("New Log File: %s\n", log_name);

    FILE * fp;
    printf("Opening new file %s\n", log_name);
    fp = fopen(log_name, "wb");
    if (fp != NULL){
        printf("New file Opened\n");
        fprintf(fp, "Hello!\nThis is Log file #%u!", log_num);
        printf("New file Written\n");
        fclose(fp);
    }
    else{
        printf("Failed to Open File\n");
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
    // Do this somewhere else V
    FILE * fp; 
    fp = fopen("/sdcard/filename.log", "w");

    fprintf(fp, " (%.4f)  %s  %lX   [%u]  ", pdTICKS_TO_MS(xTaskGetTickCount())*1000.0, "can1", message.identifier, message.data_length_code);
    for (int i = 0; i < message.data_length_code; i++) {
        fprintf(fp, "%X ", message.data[i]);
    }
    fprintf(fp, "\n");
}