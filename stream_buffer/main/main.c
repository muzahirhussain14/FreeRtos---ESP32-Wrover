/*

In FreeRTOS, a stream buffer is a thread-safe data structure designed for the efficient exchange of data between tasks or between tasks and 
interrupt service routines (ISRs). It provides a flexible and efficient mechanism for transferring blocks of data of varying lengths.

Here are some key features and characteristics of stream buffers:

    - Data Transfer: Stream buffers facilitate the transfer of a continuous stream of bytes between tasks or between a task and an ISR. 
      They allow reading and writing of data in either fixed-size blocks or smaller portions.

    - Dynamic Size: Unlike queues, stream buffers do not have a fixed size. They can dynamically grow and shrink to accommodate the data 
      being written or read, up to a specified maximum size.

    - Read/Write Operations: Stream buffers offer functions for both reading from and writing to the buffer. These functions provide various 
      options, such as blocking or non-blocking behavior, timeouts, and notification mechanisms.

    - Byte-Oriented: Stream buffers operate at the byte level, allowing individual bytes to be read or written independently. This provides 
      flexibility for applications that require byte-level access. Queues, on the other hand, deal with fixed-size elements, typically larger 
      than a single byte.

    - Data Structure: Stream buffers are implemented differently from queues. Stream buffers store data as a contiguous block, while queues 
      store data as discrete elements (such as fixed-size items).

    - Flexibility: Stream buffers offer more flexibility in terms of dynamic sizing. They can grow and shrink dynamically based on the data 
      being written or read, whereas queues have a fixed size determined during their creation.

    - Use Cases: Stream buffers are well-suited for streaming data applications, such as audio or video streaming, where a continuous stream 
      of bytes needs to be transferred. Queues, on the other hand, are often used for inter-task communication and synchronization, especially 
      for exchanging discrete items between tasks.
*/

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "esp_log.h"

static const char* TAG = "MyModule";

TaskHandle_t taskHandle;
StreamBufferHandle_t buffer;

void task()
{
    const uint32_t xStreamBufferSizeBytes = 100;              // 100 bytes
    const uint8_t xTriggerLevel = 10;
    uint8_t ucArrayToReceive[4];
    char *pcStringToSend = "Data is received by the Task";
    uint8_t xBytesReceived;
    const TickType_t xBlockTime = pdMS_TO_TICKS( 50 );

    vTaskDelay(1000 / portTICK_PERIOD_MS);              // 1 second delay  

    // Receive data
    xBytesReceived = xStreamBufferReceive( buffer,
                                    ( void * ) ucArrayToReceive,
                                    sizeof( ucArrayToReceive ),
                                    xBlockTime );

    if (xBytesReceived == 0)
        ESP_LOGI(TAG, "Task : Problem receiving data from the Buffer.");
    
    else
    {
        ESP_LOGI(TAG, "Task : Data received from the buffer. Printing: ");
        for (int i = 0; i < 4; ++i)
            ESP_LOGI(TAG, "Task : %d", ucArrayToReceive[i]);
    
    
        // Write Confirmation
        int8_t xBytesSent = xStreamBufferSend(buffer, (void *) pcStringToSend, strlen(pcStringToSend)+1, xBlockTime);       // +1 to write the null character
        if (xBytesSent != strlen(pcStringToSend)+1)
            ESP_LOGI(TAG, "Task : Problem Sending Confirmation to the Main");
        
        ESP_LOGI(TAG, "Task : Confirmation send");
    }
    
    while(1) 
    {}
}

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    ESP_LOGI(TAG, "Starting the Stream Buffer Test Program");

    // 1. Create a task
    static uint8_t ucParameterToPass; 
    const uint32_t STACK_SIZE = 2048;

    xTaskCreatePinnedToCore (task, "task_to_process_message", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &taskHandle, 0);
    configASSERT( taskHandle );


    // 2. ********** Creating the msg buffer ***********
    const uint32_t xStreamBufferSizeBytes = 100;              // 100 bytes
    const uint8_t xTriggerLevel = 10;

    size_t xBytesSent;                                        // total bytes sent
    uint8_t ucArrayToSend[] = { 0, 1, 2, 3 };                 // Array to send
    const TickType_t x100ms = pdMS_TO_TICKS( 100 );           // ticks to wait for space to become available

    // xStreamBufferSizeBytes = The total number of bytes the stream buffer will be able to hold at any one time.
    // xTriggerLevelBytes = The trigger level in a stream buffer determines the number of bytes required for a blocked task to be unblocked 
    //                      and resume execution. For example, if the trigger level is set to 1 and the task is waiting for data in an empty 
    //                      stream buffer, it will be unblocked when a single byte is written to the buffer or when the task's block time limit 
    //                      is reached.
    //                      If a task's block time expires before the trigger level is reached, the task will still receive the number of bytes 
    //                      actually available in the buffer. Setting a trigger level of 0 will effectively use a trigger level of 1. It is not 
    //                      valid to specify a trigger level greater than the buffer size.

    buffer = xStreamBufferCreate( xStreamBufferSizeBytes, xTriggerLevel );


    // 3. ********** Writing to the message Buffer ***********
    
    //  buffer = the handle of the stream buffer
    //  ucArrayToSend = data to write to the buffer (in this case an integer array)
    //  ucArrayToSend = The maximum number of bytes to copy from "ucArrayToSend" into the stream buffer 
    //  x100ms = The xTicksToWait parameter in FreeRTOS specifies the maximum time a task should remain blocked while waiting for enough 
    //           space to become available in a stream buffer. 
    //           The duration is measured in tick periods, which depend on the tick frequency of the system. You can use the pdMS_TO_TICKS() 
    //           macro to convert time from milliseconds to ticks.
    //           If xTicksToWait is set to portMAX_DELAY, the task will wait indefinitely without timing out, provided that 
    //           INCLUDE_vTaskSuspend is enabled in FreeRTOSConfig.h. If the task times out before enough space becomes available 
    //           in the buffer, it will still write as many bytes as possible.
    
    xBytesSent = xStreamBufferSend(buffer, (void *) ucArrayToSend, sizeof(ucArrayToSend), x100ms);

    if (xBytesSent != sizeof (ucArrayToSend))
    {
        ESP_LOGI(TAG, "Main : Problem writing data into the Buffer. Quiting");
        return;
    }
    else
        ESP_LOGI(TAG, "Main : Data written to the buffer.");

     vTaskDelay(3000 / portTICK_PERIOD_MS);         // delay for 3 seconds

    
    // 3. ********** Receiving from the message Buffer ***********

    uint8_t xReceivedBytes;
    char ucStringToReceive[50];

    xReceivedBytes = xStreamBufferReceive( buffer,
                                    ( void * ) ucStringToReceive,
                                    sizeof( ucStringToReceive ),
                                    x100ms );

    if (xReceivedBytes == 0 )
        ESP_LOGI(TAG, "Main : Problem receiving data from the Buffer.");
    
    else
        ESP_LOGI(TAG, "Main : Confirmation from the task received: %s", ucStringToReceive);
}
