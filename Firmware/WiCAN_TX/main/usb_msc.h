#ifndef USB_MSC_H
#define USB_MSC_H

#include "driver/sdmmc_host.h"

static const char *TAG = "USB MSC";

/* TinyUSB descriptors
   ********************************************************************* */
#define EPNUM_MSC       1
#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN)

enum {
    ITF_NUM_MSC = 0,
    ITF_NUM_TOTAL
};

enum {
    EDPT_CTRL_OUT = 0x00,
    EDPT_CTRL_IN  = 0x80,

    EDPT_MSC_OUT  = 0x01,
    EDPT_MSC_IN   = 0x81,
};

typedef enum {
    TINYUSB_STORAGE_INVALID,
    TINYUSB_STORAGE_SPI,
    TINYUSB_STORAGE_SDMMC,
    TINYUSB_STORAGE_MAX,
} tinyusb_storage_type_t;

typedef struct {
    sdmmc_host_t host;
    sdmmc_slot_config_t slot_config;
} tinyusb_config_sdmmc_t;

/**
 * @brief Configuration structure for storage initialization
 */
typedef struct {
    tinyusb_storage_type_t storage_type;
    tinyusb_config_sdmmc_t *sdmmc_config;
} tinyusb_config_storage_t;


void usb_msc_init(void);

#endif