/*
Message Buffers:

1) Fixed-size messages: Message buffers are designed to exchange fixed-sized messages between tasks.
2) Buffer organization: The buffer is divided into fixed-sized slots, with each slot accommodating one complete message.
3) Synchronization: Tasks can block on message buffers if the buffer is empty (for reading) or full (for writing).
4) Read/Write granularity: Tasks can only read or write complete messages from/to the buffer.
5) Data structure: Messages are stored contiguously in the buffer, and each message can have a predetermined size.


Stream Buffers:

1) Variable-length data: Stream buffers are designed to transfer variable-length data or streams of data between tasks.
2) Buffer organization: The buffer has a fixed overall size but can accommodate variable-length data within that size.
3) Synchronization: Tasks can block on stream buffers if the buffer is empty (for reading) or full (for writing).
4) Read/Write granularity: Tasks can read and write data in chunks of varying lengths, allowing for more flexibility in data transfer.
5) Data structure: The data within the stream buffer may not be stored contiguously, and it can be read or written in segments.
*/

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"


const static char* TAG = "MyModule";
MessageBufferHandle_t messageBufferHandle;
TaskHandle_t taskHandle;


void task(void)
{
    uint8_t bytesReceived = 0;
    uint8_t ucArraytoReceive[4];
    const TickType_t delay = pdMS_TO_TICKS(1000);

    // vTaskDelay(1000/portTICK_PERIOD_MS);

    // Receive the data sent the the Main.
    bytesReceived = xMessageBufferReceive( messageBufferHandle, ( void * ) ucArraytoReceive, sizeof(ucArraytoReceive), delay );
    if (bytesReceived != sizeof(ucArraytoReceive))
        ESP_LOGI(TAG, "TASK: Error while reading data from message buffer.");
    else
    {
        ESP_LOGI(TAG, "TASK: Data successfully read from message buffer. Data:");
        for (int i = 0; i<4; ++i)
            ESP_LOGI(TAG, "TASK: %d", ucArraytoReceive[i]);
    }

    // Writing the confirmation
    char* confirmation = "Data Successfully read from the Task";
    uint8_t bytesSent = 0;

    bytesSent = xMessageBufferSend( messageBufferHandle, (void *)confirmation, strlen(confirmation) + 1, delay);
    if (bytesSent == 0)
        ESP_LOGI(TAG, "TASK: Problem sending the confirmation.");
    else
        ESP_LOGI(TAG, "TASK: Confirmation sent Successfully.");

    while(1){}
}


void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    ESP_LOGI(TAG, "Starting the Message Buffer Test Program");

    static uint8_t ucParameterToPass;
    const int32_t STACK_SIZE = 2048;
    taskHandle = xTaskCreatePinnedToCore(task, "task_to_process_message", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &taskHandle, 0);    // for single code controllers, we can use xTaskCreate()


    // 1. Create a buffer
    
    // configSUPPORT_DYNAMIC_ALLOCATION in FreeRTOSConfig.h must be set to 1 or must be uninitialized for
    // xMessageBufferCreate method to become available.

    // size of the buffer in bytes - when a message is written to the buffer, length of the total bytes is also written at the end. Its 4 bytes 
    // on a 32-bit architecture, so on most 32-bit architectures a 10 byte message will take up 14 bytes of message buffer space.

    const size_t messageBufferSizeBytes = 104;   
    messageBufferHandle = xMessageBufferCreate( messageBufferSizeBytes );

    if (messageBufferHandle == NULL)        // buffer not created due to limited space
        return;



    // 2. Write data to the buffer
    // xMessageBufferSend(xMessageBuffer, pvTxData, xDataLengthBytes, xTicksToWait)

    uint8_t bytesSent;
    uint8_t ucArrayToSend[] = {0,1,2,3};
    const TickType_t x100ms = pdMS_TO_TICKS( 100 );

    bytesSent = xMessageBufferSend( messageBufferHandle, ( void * ) ucArrayToSend, sizeof( ucArrayToSend ), x100ms );

    if (bytesSent != sizeof( ucArrayToSend ))
    {
        // Data not sent - or atleast not all bytes sent
        ESP_LOGI(TAG, "MAIN: Unable to write the data to the message buffer.");
    }
    else
    {
        // Data sent
        ESP_LOGI(TAG, "MAIN: Data successfully written to the message buffer.");
    }


    // 3. Receive data
    uint8_t bytesReceived;
    char ucArraytoReceive[50];         

    vTaskDelay(2000/portTICK_PERIOD_MS);

    bytesReceived = xMessageBufferReceive( messageBufferHandle, ( void * ) ucArraytoReceive, sizeof(ucArraytoReceive), x100ms );
    if (bytesReceived != 0)
    {
        // Confirmation received
        ESP_LOGI(TAG, "MAIN: Data successfully read from message buffer. Data: %s", ucArraytoReceive);
    }
    else 
    {
        // Error while reading data
        ESP_LOGI(TAG, "MAIN: Error while reading data from message buffer.");
    }
}   
