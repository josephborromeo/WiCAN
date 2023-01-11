#include "sdmmc_driver.h"

#include <string.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "dirent.h"

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
    }

    sdmmc_card_print_info(stdout, card);
    print_files();

}

void print_files(void){
    DIR *dir = opendir(MOUNT_POINT);
    struct dirent *dp;

    while ((dp = readdir(dir)) != NULL) {
        printf("File found: %s\n", dp->d_name);

    }
    closedir(dir);
    printf("DIR CLOSED\n");
}