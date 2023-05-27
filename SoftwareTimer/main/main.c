/*
Software timers can be used to,
- Schedule the execution of specific tasks at predetermined intervals or after a specific delay
- Software timers can be used to implement the power-saving techniques, such as putting the system
  into a low-power sleep mode after a period of inactivity.
- Can be used to implement timeouts in communication protocols. For instance, if you are waiting for 
  a response from a peripherial device, you can start the timer and consider the operation timed out
  if the response does not arrive in a specified period. 


  In this program, wI will use software timers to wait for 5 seconds, and then create a task that will then switch on an LED.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define STACK_SIZE  2048        //Task stack size
#define BLINK_GPIO 2            // GPIO pin mapped to the led in esp32

TaskHandle_t xHandle = NULL;    // Task handle
TimerHandle_t xTimer = NULL;    // Timer handle

static const char* TAG = "MyModule";

// Function to configure the led
static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

TaskHandle_t task_to_start_led(void)
{
    configure_led();
    gpio_set_level(BLINK_GPIO, 1);              // turn on the led

    ESP_LOGV(TAG, "LED Started");

    while (1) {
        ESP_LOGI(TAG, "LED Task");
        vTaskDelay(1000 / portTICK_PERIOD_MS);              // block for 1 sec (1000ms)
    }
}

void create_task () 
{
     static uint8_t ucParameterToPass;           // parameters to pass to the task.
     void* task_to_run = (void *)pvTimerGetTimerID(xTimer);          // task to run is passed as a parameter in the timerId slot

     xTaskCreatePinnedToCore (task_to_run, "On Led", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle, 0);
     configASSERT( xHandle );

     ESP_LOGI(TAG, "Task to start an LED is created.");
}

// This is the method that creates the Software Timer
void create_swTimer()
{
    uint32_t xTimerPeriod = ( 5000 / portTICK_PERIOD_MS ) ;       // 5 seconds

    xTimer  = xTimerCreate(
        "Timer_1",                          // name of the timer
        xTimerPeriod,                       // the timer duration in ticks
        false,                              // timer will not restart when it expires. Passing True will restart the timer when it expire
        (void *)task_to_start_led,          // unique id of a timer (can also be used to pass a parameter to the callback, which is how its being used in this example )
        create_task                         // callback to call when the timer expires.
    );

    ESP_LOGI(TAG, "Timer Created.");
}

void app_main(void)
{
    // Enable logging
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);

    ESP_LOGI(TAG, "Starting the Log Test Program...");


    // Create the timer
    create_swTimer();

    // start the timer
    xTimerStart(xTimer, portMAX_DELAY);
    if (xTimerStart ( xTimer , 0) != pdPASS)
       return;                              // if timer is not started, quit
    
    ESP_LOGI(TAG, "Timer Started");
    
    while(1) {
        ESP_LOGI(TAG, "Main Program");
        vTaskDelay(1000 / portTICK_PERIOD_MS);              // block for 1 sec (1000ms)
    }
}
