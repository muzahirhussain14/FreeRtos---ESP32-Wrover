// To see the logs, I've used putty.

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


static const char* TAG = "MyModule";

void app_main(void)
{
    // Set the log level (optional)
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    // Enable logging
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    ESP_LOGV(TAG, "Starting the Log Test Program...");

    int counter = 0;
    
    while(1) 
    {
        ESP_LOGI(TAG, "\n\n(Info) Iteration: %d", counter);
        ESP_LOGW(TAG, "(Warning) Iteration: %d", counter);
        ESP_LOGD(TAG, "(Debug) Iteration: %d", counter);
        ESP_LOGE(TAG, "(Error) Iteration: %d", counter);
        ESP_LOGV(TAG, "(Verbose) Iteration: %d", counter);

        ++counter;
    }
}
