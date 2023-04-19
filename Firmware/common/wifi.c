#include "wifi.h"
#include "twai_driver.h"
#include <stdbool.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "slcan.h"

#ifdef WiCAN_RX_Board
#include "usb_driver.h"
#include "tusb_cdc_acm.h"
#endif

QueueHandle_t incoming_can_queue;

void wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE)); // No power saving
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));
    
    // FIXME: Test this out, increased RX receive rate
    // ESP_ERROR_CHECK(esp_wifi_config_espnow_rate(ESP_IF_WIFI_STA, WIFI_PHY_RATE_54M));

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
    espnow_init();
}


void espnow_init(void) {
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(espnow_recv_cb) );

    // Add Peer Device(s)
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    
    #ifdef WiCAN_TX_Board
    for (uint8_t rx=0; rx<NUM_RECEIVERS; rx++){
        memset(peer, 0, sizeof(esp_now_peer_info_t));
        peer->channel = CONFIG_ESPNOW_CHANNEL;
        peer->ifidx = ESPNOW_WIFI_IF;
        peer->encrypt = false;
        memcpy(peer->peer_addr, receiver_mac_addresses[rx], ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
        ESP_LOGI("ESP_NOW INIT", "Added Receiver %u to peer list", rx);
    }
    #endif 

    #ifdef WiCAN_RX_Board
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, transmitter_mac_address, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    ESP_LOGI("ESP_NOW INIT", "Added Transmitter to peer list");
    #endif
    free(peer);
    
}

// Send and Receive Callbacks -- Keep these short
void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{   // Just printout if error occurs - maybe disable
    
    if (mac_addr == NULL) {
        ESP_LOGI("send_cb", "Send cb arg error");
        return;
    }

    if (status==ESP_NOW_SEND_FAIL){
        ESP_LOGI("send_cb", "espnow send fail");
        // Set LED color here?
    }
}

void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int data_len){
    wican_data_t data_packet;
    memcpy(&data_packet, data, data_len);

    if (xQueueSend(incoming_can_queue, &data_packet, (TickType_t)100) != pdTRUE){
        printf("queue full\n");
    }

}

/*
    Send data to all receivers in list 
*/
void send_to_all(wican_data_t data_packet) {
    static esp_err_t ret = ESP_OK;
    static uint8_t err_count = 0;
    static uint32_t err_timer;
    static bool send_ok = true;                                                                                      
    // TODO: Get return value of esp_now_send and trigger LED light to change state -- should be able to do this in send_cb

    if (true){
        for (uint8_t rx=0; rx<NUM_RECEIVERS; ++rx){
            ret = esp_now_send(receiver_mac_addresses[rx], (uint8_t*)&data_packet, sizeof(data_packet));    // Change this so instead of using all receiver mac addresses, we only use the ones we connected to - populate another list/ vec 
        }
        if (ret != ESP_OK){
            err_count++;
            // ESP_LOGI("send", "%s", esp_err_to_name(ret));
        }
    }

    else{
        if (pdTICKS_TO_MS(xTaskGetTickCount()) - err_timer > 5000){     // Wait 5 seconds before retrying after failing
            send_ok = true;
        }
    }

    if (err_count > 50){
        err_timer = pdTICKS_TO_MS(xTaskGetTickCount());
        send_ok = false;
        err_count = 0;
    }

}


// TODO: Fix this to support new format
void send_CAN_frame_to_Tx(twai_message_t message){
    wican_data_t data_packet = {0};
    data_packet.num_frames = 1;

    data_packet.frames[0].extd = message.extd;
    data_packet.frames[0].identifier = message.identifier;
    data_packet.frames[0].data_length_code = message.data_length_code;
    memcpy(data_packet.frames[0].data, message.data, sizeof(message.data));

    esp_now_send(transmitter_mac_address, (uint8_t*)&data_packet, sizeof(data_packet));
}

// Only receiver should print out received message, transmitter should transmit on bus
void parse_incoming(void *){
    incoming_can_queue = xQueueCreate(INCOMING_MSG_QUEUE_SIZE, sizeof(wican_data_t));
    uint8_t msg_buffer[SLCAN_MTU]; // Max Length of SLCAN Message
    static uint32_t rcv_counter = 0;
    uint32_t last_time = pdTICKS_TO_MS(xTaskGetTickCount());
    static uint32_t diff = 0;
    char *scan_buf = (char *)malloc(64);
    size_t length = 0; 

    while (1) {
        wican_data_t data_packet = {0};
        if( xQueueReceive(incoming_can_queue, &(data_packet), (TickType_t)1) == pdTRUE) {
            // printf("num_frames: %u\n", data_packet.num_frames);
            for (uint8_t i=0; i<data_packet.num_frames; i++){
                twai_message_t message;
                message.extd = data_packet.frames[i].extd;
                message.identifier = data_packet.frames[i].identifier;
                message.data_length_code = data_packet.frames[i].data_length_code;
                memcpy(message.data, data_packet.frames[i].data, sizeof(data_packet.frames[i].data));

                #ifdef WiCAN_RX_Board
                // rcv_counter++;

                // diff = pdTICKS_TO_MS(xTaskGetTickCount()) - last_time;
                // if (diff >= 1000){  // Print message every second
                //     printf("%.2f msgs / second\n", (((float)rcv_counter/diff)*1000.0));
                //     // length = sprintf(scan_buf, "%.1fmsgs/s\n>", (((float)rcv_counter/diff)*1000.0));
                // //     // tinyusb_cdcacm_write_queue(0, (uint8_t*)scan_buf, length);
                // //     // tinyusb_cdcacm_write_flush(0, portMAX_DELAY);

                //     rcv_counter = 0;
                //     last_time = pdTICKS_TO_MS(xTaskGetTickCount());
                    
                // }
                uint8_t length = slcan_format((uint8_t *)&msg_buffer, message);
                write_to_usb((uint8_t *)&msg_buffer, length);

                // printf(" (%.4f)  %s  %lX   [%u] ", pdTICKS_TO_MS(xTaskGetTickCount())/1000.0, "can1", message.identifier, message.data_length_code);
                // for (int i = 0; i < message.data_length_code; i++) {
                //     printf(" %02X", message.data[i]);
                // }

                // printf("\n");

                #endif

                #ifdef WiCAN_TX_Board
                // Send received CAN message on the bus
                xQueueSend(tx_can_queue, &data_packet, (TickType_t)0);      // FIXME: support new format
                #endif

            }
            #ifdef WiCAN_RX_Board
            flush_usb();
            #endif
        }
    } 
}