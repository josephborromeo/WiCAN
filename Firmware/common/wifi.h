#include <stdbool.h>
#include <string.h>
// #include <stdio.h>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_now.h"


#define CONFIG_ESPNOW_ENABLE_LONG_RANGE false       // TODO: Change to true

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA
#define CONFIG_ESPNOW_CHANNEL 1

// Transmitter MAC Address
static uint8_t transmitter_mac_address[ESP_NOW_ETH_ALEN] = {0x70, 0x04, 0x1D, 0xA7, 0xF7, 0x94};

/* ~~~  Receiver WiCAN Configuration  ~~~ */
#define NUM_RECEIVERS   (uint8_t)1
static uint8_t receiver_mac_addresses[NUM_RECEIVERS][ESP_NOW_ETH_ALEN] = { 
    {0x70, 0x04, 0x1D, 0xA7, 0xF7, 0x8C},
    };

// Custom packet datastructure

typedef struct{
    char text[20];        // FIXME: Change to pointers and use malloc for string insertion
} wican_data;

void wifi_init(void);
void espnow_init(void);
void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int data_len);

void send_data_task(void);