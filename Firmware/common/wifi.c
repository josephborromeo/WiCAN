#include "wifi.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

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
    
    // FIXME: Might be causing crashing when there is no receiver
    
    if (mac_addr == NULL) {
        ESP_LOGI("send_cb", "Send cb arg error");
        return;
    }

    if (status==ESP_NOW_SEND_FAIL){
        // ESP_LOGI("send_cb", "espnow send fail");
        // Set LED color here?
    }
}

void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int data_len){
    wican_data_t recv_data;
    memcpy(&recv_data, data, data_len);
    xQueueSend(incoming_can_queue, &recv_data, (TickType_t)0); 
}

/*
    Send data to all receivers in list 
*/
void send_to_all(const uint8_t *data, size_t len) {
    // TODO: Get return value of esp_now_send and trigger LED light to change state -- should be able to do this in send_cb
    for (uint8_t rx=0; rx<NUM_RECEIVERS; ++rx){
        esp_now_send(receiver_mac_addresses[rx], data, len);    // Change this so instead of using all receiver mac addresses, we only use the ones we connected to - populate another list/ vec 
    }
}

void test_send_data_task(void*){
    vTaskDelay(pdMS_TO_TICKS(1000));
    twai_message_t message;
    
    message.identifier = 1234;
    message.extd=1;
    message.data_length_code = 8;

    for (int i = 0; i < message.data_length_code; i++) {
        message.data[i] = i;
    }


    // const uint8_t *data = 64;

    while (1){
        send_CAN_frame(message);
        // send_to_all(data, sizeof(data));
        vTaskDelay(pdMS_TO_TICKS(500));    // Sleep for 0.5s
    }
}

void send_CAN_frame(twai_message_t message){
    wican_data_t can_data;
    can_data.data_type = CAN_FRAME;
    can_data.data = message;

    send_to_all((uint8_t*)&can_data, sizeof(can_data));
}


// Make this a task that runs
void parse_incoming(void *){
    incoming_can_queue = xQueueCreate(INCOMING_MSG_QUEUE_SIZE, sizeof(twai_message_t));
    wican_data_t received_data;
    while (1) {
        if( xQueueReceive(incoming_can_queue, &(received_data), (TickType_t)portMAX_DELAY)) {
            switch (received_data.data_type)
            {
                case CAN_FRAME:
                twai_message_t message = received_data.data;
                printf("ID: %lu\tExt ID: %i\tDLC:%u\t", message.identifier, message.extd, message.data_length_code);
                for (int i = 0; i < message.data_length_code; i++) {
                    printf("%d ", message.data[i]);
                }
                printf("\n");
                break;

                default:
                ESP_LOGI("WIFI_PARSE", "Unknown Incoming Message Format");
            }
        }
    }
    
}