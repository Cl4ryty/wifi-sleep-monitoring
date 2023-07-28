#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"


// how many raw 802.11 frames to send per second ~equal to the received CSI frames at the AP
#define PACKETS_PER_SECOND CONFIG_FRAMES_PER_SECOND

static const char *TAG = "csi sender";

// see https://github.com/espressif/esp-idf/blob/ae425ec0b877cdaab12835385bb441d59704e261/docs/en/api-guides/wifi.rst#wi-fi-80211-packet-send

uint8_t raw_null_data_frame_buffer[] = {    // raw data frame
	0x48, 0x81,							    // 0-1: Frame Control, subtype 0100 (null data), type 10 (data frame), toDS=1, order=1, all other 0
	0x00, 0x00,							    // 2-3: Duration
	0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,		// 4-9: Address 1 → RA=DA → the receiver address → of the AP
	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,		// 10-15: Address 2 → TA=BSSID → the address of the transmitter → this STA
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,		// 16-21 Address 2 → SA → the address of the entity that initiated the frame → this STA
	0x00, 0x00,							    // 22-23: Sequence cotrol
    
};

static void frame_send_task(void *pvParameters)
{
    while(1)
    {
        ESP_LOGI(TAG, "csi");
        esp_wifi_80211_tx(WIFI_IF_STA, raw_null_data_frame_buffer, sizeof(raw_null_data_frame_buffer), true);
        vTaskDelay( (1000/PACKETS_PER_SECOND) /  portTICK_PERIOD_MS);
    }
}

void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {},
    };

    // disable modem sleep
    esp_wifi_set_ps(WIFI_PS_NONE);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA finished");

    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11N);
    esp_wifi_config_80211_tx_rate(WIFI_IF_STA, WIFI_PHY_RATE_MCS7_SGI);
    esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);

    ESP_LOGI(TAG, "csi start");
    xTaskCreate(frame_send_task, "frame_send", 4096, NULL, 5, NULL);  
}
