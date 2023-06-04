#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "example";                    // For Logging
QueueHandle_t xQueue = NULL;                           // Queue Handle
TaskHandle_t xTask = NULL;

#define STACK_SIZE  2048

struct Message 
{
    char messageId;
    char data[20];
};

// Queue
bool CreateQueue() 
{
    xQueue = xQueueCreate( 10, sizeof( struct Message * ) );           // Queue that can hold 10 pointers of Message Struct
    if (xQueue == 0)
        return false;                                                  // Failed to create a Queue
    
    return true;
}

// Create Task
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
    xTaskCreatePinnedToCore (task_to_run, "Receive Message", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle, 0);
    configASSERT( xHandle );

    return xHandle;
}

void Task(void* pvParameters)
{
    while(xQueue == 0 || xQueue == NULL);                           // Wait until the data is written in the queue
    
    struct Message msg;

    // Receive data from the struct
    if (xQueueReceive(xQueue, &(msg), ( TickType_t ) 10))           // if successful read
    {
        // Print the contents
        ESP_LOGI(TAG, "Data is received in the Thread. Printing Contents:");
        ESP_LOGI(TAG, "Message ID %c:", msg.messageId);
        ESP_LOGI(TAG, "Message %s:", msg.data);
    }

    while (1) {}
}


void app_main(void)
{
    ESP_LOGI(TAG, "Starting the Program.");

    xQueue = NULL;
    bool queueCreatedSuccessfull = false;
    struct Message msg;

    // Create a task to receive the msg
    xTask = create_task(Task);

    while(!queueCreatedSuccessfull)                     // keep trying to create a queue until successfully created.
    {
        queueCreatedSuccessfull = CreateQueue();
    }

    msg.messageId = 'S';
    memcpy(msg.data, "Hello World", 11 * sizeof(char)); // dst, src, size in bytes to write 
    


    // write struct to the Queue which will then be read by the thread
    
    // Parameters
    // 1) Handle to the Queue
    // 2) address of the msg struct
    // 3) The maximum amount of time the task should block waiting for space to become available on the queue, should it already be full. (10 ticks)
    // 4) Place the item to the back of the queue, as oppose to queueSEND_TO_FRONT 
    
    xQueueGenericSend( xQueue, ( void * ) &msg, ( TickType_t ) 10, queueSEND_TO_BACK );

    ESP_LOGI(TAG, "Data is sent from the Main thread.");

    // Main loop   
    while (1) {

    }
}
