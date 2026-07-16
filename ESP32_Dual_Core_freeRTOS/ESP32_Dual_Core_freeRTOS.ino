/**********************************************************************
*
 * Main Application
 *
 **********************************************************************/

#include "Config.h"
#include "UART_Manager.h"
#include "HTTP_Server.h"
#include "Download_Manager.h"
#include "WebUI.h"

DownloadStatus_t DownloadStatus;
uint8_t UART_Buffer[UART_BUFFER_SIZE];

TaskHandle_t HTTP_TaskHandle = NULL;
TaskHandle_t UART_TaskHandle = NULL;
QueueHandle_t data_queue;

void setup()
{
    Serial.begin(115200);

    while (!Serial)
        delay(10);

    Download_Init();
    UART_Manager_Init();

    if (!HTTP_Server_Init())
    {
        DBGLN("HTTP Server Initialization Failed");
        while (1)
        {
            delay(1000);
        }
    }

    data_queue = xQueueCreate(5, sizeof(UART_Packet_t));

    xTaskCreatePinnedToCore(HTTP_Server_Task, "HTTP_Server", 16384, NULL, 2, &HTTP_TaskHandle, 0);
    xTaskCreatePinnedToCore(UART_Manager_Task, "UART_Manager", 16384, NULL, 2, &UART_TaskHandle, 1);

    DBGLN("System Ready");
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}