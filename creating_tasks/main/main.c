// Reference:  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html#_CPPv423xTaskCreatePinnedToCore14TaskFunction_tPCKcK22configSTACK_DEPTH_TYPEPCv11UBaseType_tPC12TaskHandle_tK10BaseType_t

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define STACK_SIZE  2048    //Task stack size
#define BLINK_GPIO 2        // GPIO pin mapped to the led in esp32

// Function to configure the led
static void configure_led(void)
{
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


// task 1
void on_led(void* pvParameters)
{
    gpio_set_level(BLINK_GPIO, 1);          // turn on the led
    while (1) {}

}

// task 2
void off_led(void* pvParameters)
{
    gpio_set_level(BLINK_GPIO, 0);          // turn of the led
    while (1) {}

}

TaskHandle_t create_task (void (*task_to_run)()) 
{
    static uint8_t ucParameterToPass;           // parameters to pass to the task.
    TaskHandle_t xHandle = NULL;                // task handle


    // Create the task pinned to core 0, storing the handle.  Note that the passed parameter ucParameterToPass
    // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
    // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
    // the new task attempts to access it.
    
    // This function is similar to xTaskCreate (used for single core controllers), but allows setting task affinity in SMP (Symmetric Multiprocessing) system.
    // This task will be pinned to core 0 (last parameter). If you don't want to specify the affinity pass tskNO_AFFINITY in the last parameter.
    xTaskCreatePinnedToCore (task_to_run, "On Led", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle, 0);
    configASSERT( xHandle );

    return xHandle;
}


// main function
void app_main(void)
{
    TaskHandle_t xHandle = NULL;
    configure_led();

    while(1) 
    {
        // create a task to turn on an led

        xHandle = create_task(on_led);
        vTaskDelay(pdMS_TO_TICKS(5000));          // Delay a task for a given number of ticks (5000 ticks = 5 seconds)

        if( xHandle != NULL )
        {
            vTaskDelete( xHandle );
            xHandle = NULL;
        }


        // create a second task to turn off the led
        xHandle = create_task(off_led);
        vTaskDelay(pdMS_TO_TICKS(5000));          // Delay this task for 5 seconds and then delete this task

        if( xHandle != NULL )
        {
            vTaskDelete( xHandle );
            xHandle = NULL;
        }
    }
}
