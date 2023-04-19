#include "usb_driver.h"

#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "string.h"
#include "driver/twai.h"
#include "slcan.h"

#define ITF_NUM_CDC    0

static const char *TAG = "USB";
static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];


// Write buffer to usb output
void write_to_usb (uint8_t* tx_buffer, uint8_t buff_size){
    tinyusb_cdcacm_write_queue(ITF_NUM_CDC, tx_buffer, buff_size);
    tinyusb_cdcacm_write_flush(ITF_NUM_CDC, 1000);
}

void send_cr(){
    uint8_t cr_buf[1];
    cr_buf[0] = '\r';
    tinyusb_cdcacm_write_queue(ITF_NUM_CDC, cr_buf, 1);
    tinyusb_cdcacm_write_flush(ITF_NUM_CDC, 0);
}

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    /* initialization */
    size_t rx_size = 0;

    /* read */
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Data from channel %d:", itf);
        ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
    } else {
        ESP_LOGE(TAG, "Read error");
    }

    /* write back */
    // printf("Buffer: %s\n", buf);
    // tinyusb_cdcacm_write_queue(ITF_NUM_CDC, buf, rx_size);
    // tinyusb_cdcacm_write_flush(ITF_NUM_CDC, 0);
    // write_to_usb((uint8_t*)&buf);
    
    // FIXME: Currently only returns a carriage return (\r)
    twai_message_t message;

    int8_t ret_err;
    ret_err = slcan_parse_str(buf, rx_size, &message);

    send_cr();
}



void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

void usb_init(void){
    ESP_LOGI("USB Driver", "USB initialization");

    tusb_desc_device_t my_descriptor = {
        .bLength = sizeof(my_descriptor),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0200, // USB version. 0x0200 means version 2.0
        .bDeviceClass = TUSB_CLASS_UNSPECIFIED,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = 0x303A,
        .idProduct = 0x3000,
        .bcdDevice = 0x0101, // Device FW version

        .iManufacturer = 0x01, // see string_descriptor[1] bellow
        .iProduct = 0x02,      // see string_descriptor[2] bellow
        .iSerialNumber = 0x03, // see string_descriptor[3] bellow

        .bNumConfigurations = 0x01};

    tusb_desc_strarray_device_t my_string_descriptor = {
        // array of pointer to string descriptors
        (char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
        "Formula Electric",                  // 1: Manufacturer
        "WiCAN Receiver",   // 2: Product
        "012-345",            // 3: Serials, should use chip ID
    };


    // const tinyusb_config_t tusb_cfg = {
    //     .device_descriptor = &my_descriptor,
    //     .string_descriptor = my_string_descriptor,
    //     .external_phy = false,
    //     // .configuration_descriptor = NULL,
    // };

    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };

    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));

    ESP_LOGI("USB Driver", "USB initialization DONE");


}
