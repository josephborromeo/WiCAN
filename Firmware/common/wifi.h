#include <stdbool.h>
#include <string.h>
// #include <stdio.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "driver/twai.h"

#include "../common/data_types.h"


#define CONFIG_ESPNOW_ENABLE_LONG_RANGE true       // TODO: Change to true

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#define CONFIG_ESPNOW_CHANNEL 6

#define INCOMING_MSG_QUEUE_SIZE 10

// Transmitter MAC Address
static uint8_t transmitter_mac_address[ESP_NOW_ETH_ALEN] = {0x70, 0x04, 0x1D, 0xA7, 0xF7, 0x94};

/* ~~~  Receiver WiCAN Configuration  ~~~ */
#define NUM_RECEIVERS   (uint8_t)1
static uint8_t receiver_mac_addresses[NUM_RECEIVERS][ESP_NOW_ETH_ALEN] = { 
    {0x70, 0x04, 0x1D, 0xA7, 0xF7, 0x8C},
    // {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    };

void wifi_init(void);
void espnow_init(void);
void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int data_len);

void test_send_data_task(void*);
void send_to_all(const uint8_t *data, size_t len);

void send_CAN_frame(twai_message_t message);
void send_temp_data(void*);

void parse_incoming(void *);
