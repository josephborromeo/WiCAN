#include "wifi.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

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
    
    uint8_t mac[6];
    esp_wifi_get_mac(ESPNOW_WIFI_IF, mac);
    ESP_LOGI("WIFI", "MAC Address: %X:%X:%X:%X:%X:%X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

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
    
    #ifdef WICAN_TX
    for (uint8_t rx=0; rx<NUM_RECEIVERS; ++rx){
        memset(peer, 0, sizeof(esp_now_peer_info_t));
        peer->channel = CONFIG_ESPNOW_CHANNEL;
        peer->ifidx = ESPNOW_WIFI_IF;
        peer->encrypt = false;
        memcpy(peer->peer_addr, receiver_mac_addresses[rx], ESP_NOW_ETH_ALEN);
        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    }
    #endif 

    #ifdef WICAN_RX
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    memcpy(peer->peer_addr, transmitter_mac_address, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    #endif
    free(peer);
    ESP_LOGI("ESP_NOW INIT", "Added Transmitter to peer list");
}

// Send and Receive Callbacks -- Keep these short

void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{   
    ESP_LOGI("send_cb", "cb ran");
    if (mac_addr == NULL) {
        ESP_LOGI("send_cb", "Send cb arg error");
        return;
    }

    if (status==ESP_NOW_SEND_FAIL){
        ESP_LOGI("send_cb", "espnow send fail");
    }
}

void espnow_recv_cb(const uint8_t *mac, const uint8_t *data, int data_len){
    wican_data *recv_data = data;
	ESP_LOGI("RECV_CB", "DATA: %p", recv_data->text);
}


void send_data_task(void){
    wican_data *sending_data = {"Hello World"};

    while (1){

        for (uint8_t rx=0; rx<NUM_RECEIVERS; ++rx){
            ESP_ERROR_CHECK(esp_now_send(receiver_mac_addresses[rx], sending_data, sizeof(sending_data)));
            ESP_LOGI("SEND_FUNC", "Sent Data Via ESP NOW");
            // ESP_LOGI("SEND_FUNC", "%u", ret);
            
        }
        

        vTaskDelay(pdMS_TO_TICKS(1000));    // Sleep for 0.5s
    }
}

