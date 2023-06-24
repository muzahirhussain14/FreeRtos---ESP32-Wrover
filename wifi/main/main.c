#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

static const char* TAG = "Wifi-example";
esp_netif_t *wifi_sta_netif;

static esp_err_t event_handler(void *ctx, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        ESP_LOGI(TAG, "Wi-Fi started");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) 
    {
        ESP_LOGI(TAG, "Wi-Fi connected");
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        ESP_LOGI(TAG, "Wi-Fi disconnected");
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP address");
        ESP_LOGI(TAG, "IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
    }

    return ESP_OK;
}

void wifi_init_sta(void)
{
    // initialize the Non-Volatile Storage (NVS). NVS is a key-value storage system that allows you to store and 
    // retrieve configuration data that persists even after the device is powered off or reset.
    ESP_ERROR_CHECK(nvs_flash_init());      
    
    // Initializes the network interface data structures
    ESP_ERROR_CHECK(esp_netif_init());

    // The function is used to create the default event loop in ESP-IDF. The event loop is responsible for handling and dispatching
    // events that occur within the ESP-IDF framework.
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // esp_netif_create_default_wifi_sta is used to create the default network interface for the Wi-Fi station (STA) mode.
    // When you create the default Wi-Fi station network interface, it becomes the primary interface for connecting to Wi-Fi networks 
    // in station mode. 
    // You can use this interface to configure Wi-Fi settings, connect to access points, and perform other Wi-Fi-related operations.
    wifi_sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    // Initialize the wifi driver with the provided configuration. 
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "BTHub6-2G2K",
            .password = "JGQ6d64xVNPm"
        },
    };

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize Wi-Fi
    wifi_init_sta();

    esp_netif_ip_info_t ip_info;
    while(1) 
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_err_t ret = esp_netif_get_ip_info(wifi_sta_netif, &ip_info);


         if (ret == ESP_OK) 
         {
            // IP information retrieved successfully
            ESP_LOGI("", "\n");
            ESP_LOGI(TAG, "IP Address: " IPSTR, IP2STR(&ip_info.ip));
            ESP_LOGI(TAG, "Subnet Mask: " IPSTR,  IP2STR(&ip_info.netmask));
            ESP_LOGI(TAG, "Gateway: " IPSTR,  IP2STR(&ip_info.gw));
        }
        else
            // Failed to retrieve IP information
            printf("Failed to get IP information\n");
    };
}