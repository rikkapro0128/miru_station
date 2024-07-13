#pragma once

// Common library
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
// include the WiFi library
#include "esp_wifi.h"
#include "nvs_flash.h"
// Include freeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <string>
#include <iostream>
#include <functional>

#include "lwip/err.h"
#include "lwip/sys.h"

typedef enum {
    MIRU_WIFI_EVENT_STA_START,
    MIRU_WIFI_EVENT_STA_STOP,
    MIRU_WIFI_EVENT_STA_GOT_IP,
} miru_wifi_event_t;

void startWifi(std::string ssid, std::string password);
void stopWifi(void);
void onWifiEvent(miru_wifi_event_t event, void *custome_data);
void registerWifiEventCallback(std::function<void(miru_wifi_event_t, void*)> callback);