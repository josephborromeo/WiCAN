#include "usb_driver.h"

#include "tinyusb.h"
// #include "tusb_cdc_acm.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "string.h"

void usb_init(void){
    ESP_LOGI("USB Driver", "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    // tinyusb_config_cdcacm_t acm_cfg = {
    //     .usb_dev = TINYUSB_USBDEV_0,
    //     .cdc_port = TINYUSB_CDC_ACM_0,
    //     .rx_unread_buf_sz = 64,
    //     .callback_rx = NULL, // the first way to register a callback
    //     .callback_rx_wanted_char = NULL,
    //     .callback_line_state_changed = NULL,
    //     .callback_line_coding_changed = NULL
    // };

    // ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    /* the second way to register a callback */
    // ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
    //                     TINYUSB_CDC_ACM_0,
    //                     CDC_EVENT_LINE_STATE_CHANGED,
    //                     &tinyusb_cdc_line_state_changed_callback));

    // #if (CONFIG_TINYUSB_CDC_COUNT > 1)
    // acm_cfg.cdc_port = TINYUSB_CDC_ACM_1;
    // ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    // ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
    //                     TINYUSB_CDC_ACM_1,
    //                     CDC_EVENT_LINE_STATE_CHANGED,
    //                     &tinyusb_cdc_line_state_changed_callback));
    // #endif

    ESP_LOGI("USB Driver", "USB initialization DONE");


}


// static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
// void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
// {
//     /* initialization */
//     size_t rx_size = 0;

//     /* read */
//     esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
//     if (ret == ESP_OK) {
//         ESP_LOGI("USB Driver", "Data from channel %d:", itf);
//         ESP_LOG_BUFFER_HEXDUMP("USB Driver", buf, rx_size, ESP_LOG_INFO);
//     } else {
//         ESP_LOGE("USB Driver", "Read error");
//     }

//     /* write back */
//     tinyusb_cdcacm_write_queue(itf, buf, rx_size);
//     tinyusb_cdcacm_write_flush(itf, 0);
// }


const char* slcan_format(twai_message_t message){
    char output_string[19]; // Max Length of SLCAN Message
    // Start of Frame
    strcat(output_string, SOF);

    // Timestamp

    // DLC

    // Arbitration ID

    // Payload (Data)

    // End of Frame
    strcat(output_string, EOF);

    return output_string;
}