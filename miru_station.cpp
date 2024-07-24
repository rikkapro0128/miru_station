#include "miru_station.hpp"

const char *TAG = "MiruStation";
static int s_retry_num = 0;
static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;
// callback func for wifi event
static std::function<void(miru_wifi_event_t, void*)> wifi_event_callback = nullptr;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        if(wifi_event_callback != nullptr) {
            wifi_event_callback(MIRU_WIFI_EVENT_STA_START, nullptr);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // ESP_LOGI(TAG,"Connect to the AP fail");
        esp_wifi_connect();
        s_retry_num++;
        // ESP_LOGI(TAG, "retry to connect to the AP...");
        if(wifi_event_callback != nullptr) {
            wifi_event_callback(MIRU_WIFI_EVENT_STA_STOP, nullptr);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        // ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;

        if(wifi_event_callback != nullptr) {
            wifi_event_callback(MIRU_WIFI_EVENT_STA_GOT_IP, event);
        }
    }
}

void registerWifiEventCallback(std::function<void(miru_wifi_event_t, void*)> callback)
{
    wifi_event_callback = callback;
}

void startWifi(std::string ssid, std::string password)
{
    ESP_LOGI(TAG, "Starting WiFi...");

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config;

    memset(&wifi_config, 0, sizeof(wifi_config));
    memcpy(wifi_config.sta.ssid, ssid.c_str(), ssid.length());
    memcpy(wifi_config.sta.password, password.c_str(), password.length());

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi started.");
}

void stopWifi(void)
{
    ESP_LOGI(TAG, "Stopping WiFi...");

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

    ESP_ERROR_CHECK(esp_wifi_stop());

    ESP_LOGI(TAG, "WiFi stopped.");
}
