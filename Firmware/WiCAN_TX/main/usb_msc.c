/*
USB Mass Storage Device (MSC)
Example: https://github.com/espressif/esp-idf/tree/master/examples/peripherals/usb/device/tusb_msc
*/

#include "usb_msc.h"

#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "sdkconfig.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "class/msc/msc_device.h"
#include "tusb_msc_storage.h"
#include "esp_log.h"

#include "sdmmc_driver.h"


static uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 300),

    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 0, EDPT_MSC_OUT, EDPT_MSC_IN, TUD_OPT_HIGH_SPEED ? 512 : 64),
};

static tusb_desc_device_t descriptor_config = {
    .bLength = sizeof(descriptor_config),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = 0x303A,
    .idProduct = 0x4002,
    .bcdDevice = 0x100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

static char const *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },  // 0: is supported language is English (0x0409)
    "TinyUSB",                      // 1: Manufacturer
    "TinyUSB Device",               // 2: Product
    "123456",                       // 3: Serials
    "Example MSC",                  // 4. MSC
};

const tinyusb_config_t tusb_cfg = {
    .device_descriptor = &descriptor_config,
    .string_descriptor = string_desc_arr,
    .external_phy = false,
    .configuration_descriptor = desc_configuration,
};

/*      Mandatory Callback functions for USB MSC     */
/** SCSI ASC/ASCQ codes. **/
/** User can add and use more codes as per the need of the application **/
#define SCSI_CODE_ASC_MEDIUM_NOT_PRESENT 0x3A /** SCSI ASC code for 'MEDIUM NOT PRESENT' **/
#define SCSI_CODE_ASC_INVALID_COMMAND_OPERATION_CODE 0x20 /** SCSI ASC code for 'INVALID COMMAND OPERATION CODE' **/
#define SCSI_CODE_ASCQ 0x00

// static void _mount(void);
// static void _unmount(void);
static bool is_eject = false;

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    (void) lun;
    ESP_LOGD(TAG, "tud_msc_inquiry_cb() invoked");

    const char vid[] = "TinyUSB";
    const char pid[] = "Flash Storage";
    const char rev[] = "0.1";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void) lun;
    ESP_LOGD(TAG, "tud_msc_test_unit_ready_cb() invoked");

    if (is_eject) {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, SCSI_CODE_ASC_MEDIUM_NOT_PRESENT, SCSI_CODE_ASCQ);
        return false;
    } else {
        ESP_LOGD(TAG, "tud_msc_test_unit_ready_cb: MSC START: Expose Over USB");
        // _unmount();
        return true;
    }
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void) lun;

    uint32_t sec_count = storage_get_sector_count();
    uint32_t sec_size = storage_get_sector_size();
    ESP_LOGD(TAG, "tud_msc_capacity_cb() sec_count(%lu), sec_size(%lu)", sec_count, sec_size);
    *block_count = sec_count;
    *block_size  = (uint16_t)sec_size;
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void) lun;
    (void) power_condition;
    ESP_LOGI(TAG, "tud_msc_start_stop_cb() invoked, power_condition=%d, start=%d, load_eject=%d", power_condition, start, load_eject);

    if (load_eject && !start) {
        is_eject = true;
        ESP_LOGI(TAG, "tud_msc_start_stop_cb: MSC EJECT: Mount on Example");
        // _mount();
    }
    return true;
}

// Invoked when received SCSI READ10 command
// - Address = lba * BLOCK_SIZE + offset
// - Application fill the buffer (up to bufsize) with address contents and return number of read byte.
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    ESP_LOGD(TAG, "tud_msc_read10_cb() invoked, lun=%d, lba=%lu, offset=%lu, bufsize=%lu", lun, lba, offset, bufsize);

    esp_err_t err = storage_read_sector(lba, offset, bufsize, buffer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "storage_read_sector failed: 0x%x", err);
        return 0;
    }
    return bufsize;
}

// Invoked when received SCSI WRITE10 command
// - Address = lba * BLOCK_SIZE + offset
// - Application write data from buffer to address contents (up to bufsize) and return number of written byte.
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    ESP_LOGD(TAG, "tud_msc_write10_cb() invoked, lun=%d, lba=%lu, offset=%lu, bufsize=%lu", lun, lba, offset, bufsize);

    esp_err_t err = storage_write_sector(lba, offset, bufsize, buffer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "storage_write_sector failed: 0x%x", err);
        return 0;
    }
    return bufsize;
}

/**
 * Invoked when received an SCSI command not in built-in list below.
 * - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, TEST_UNIT_READY, START_STOP_UNIT, MODE_SENSE6, REQUEST_SENSE
 * - READ10 and WRITE10 has their own callbacks
 *
 * \param[in]   lun         Logical unit number
 * \param[in]   scsi_cmd    SCSI command contents which application must examine to response accordingly
 * \param[out]  buffer      Buffer for SCSI Data Stage.
 *                            - For INPUT: application must fill this with response.
 *                            - For OUTPUT it holds the Data from host
 * \param[in]   bufsize     Buffer's length.
 *
 * \return      Actual bytes processed, can be zero for no-data command.
 * \retval      negative    Indicate error e.g unsupported command, tinyusb will \b STALL the corresponding
 *                          endpoint and return failed status in command status wrapper phase.
 */
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    int32_t ret;

    ESP_LOGD(TAG, "tud_msc_scsi_cb() invoked. bufsize=%d", bufsize);

    switch (scsi_cmd[0]) {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        /* SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL is the Prevent/Allow Medium Removal
        command (1Eh) that requests the library to enable or disable user access to
        the storage media/partition. */
        ESP_LOGI(TAG, "tud_msc_scsi_cb() invoked: SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL");
        ret = 0;
        break;
    default:
        ESP_LOGW(TAG, "tud_msc_scsi_cb() invoked: %d", scsi_cmd[0]);
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, SCSI_CODE_ASC_INVALID_COMMAND_OPERATION_CODE, SCSI_CODE_ASCQ);
        ret = -1;
        break;
    }
    return ret;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    is_eject = true;
    ESP_LOGI(TAG, "tud_umount_cb: Mount on Example");
    // _mount();
}

// Invoked when device is mounted (configured)
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "tud_mount_cb MSC START: Expose Over USB");
    // _unmount();
}

/*      -----------------------------------------    */

void usb_msc_init(void) {
    ESP_LOGI("USB MSC", "Initializing storage...");
    
    tinyusb_config_storage_t storage_config;
    // tinyusb_config_sdmmc_t config_sdmmc;

    storage_config.storage_type = TINYUSB_STORAGE_SDMMC;
    
    // // From sdmmc_driver.h
    // config_sdmmc.host = &host;
    // config_sdmmc.slot_config = &slot;

    // storage_config.sdmmc_config = &config_sdmmc;

    tinyusb_driver_install(&tusb_cfg);
}
