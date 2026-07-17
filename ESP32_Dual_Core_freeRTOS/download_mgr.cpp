/**********************************************************************
 *
 * File        : download_mgr.cpp
 *
 *********************************************************************/
#include "TestPDF_Array.h"
#include "Download_Manager.h"
#include "UART_Manager.h"
#include "Config.h"

static bool DownloadBusy = false;
static uint32_t TotalSent = 0;

void Download_Init(void)
{
    DownloadBusy = false;
}

bool Download_IsBusy(void)
{
    return DownloadBusy;
}

void Download_Stop(void)
{
    DownloadBusy = false;
}

bool Download_Start(WiFiClient &client)
{
    uint32_t length;
    uint32_t lastReceiveTime;

    if (DownloadBusy)
    {
        client.println("HTTP/1.1 409 Conflict");
        client.println();
        return false;
    }
    DownloadBusy = true;

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/pdf");
    client.print("Content-Disposition: attachment; filename=\"");
    client.print(DOWNLOAD_FILENAME);
    client.println("\"");
    client.println("Connection: close");
    client.println();

    lastReceiveTime = millis();

    UART_Packet_t DataRxPacket;
    while(client.connected())
    {
        if (xQueueReceive(data_queue, (void *)&DataRxPacket, 1) == pdTRUE) 
        {
            size_t written = client.write(DataRxPacket.Data,DataRxPacket.Length);
            TotalSent += written;
        }
        vTaskDelay(1);
        if(ReportFinished && uxQueueMessagesWaiting(data_queue) == 0)
        {
            uint32_t t3 = millis();
            DBGF("Total Time : %ld\n", t3);
            DBGLN("PDF Transfer Finished");
            UART_SendCommand(CMD_ACK_END);
            ReportFinished = false;
            break;
        }
    }

    client.flush();
    client.stop();
    DBGF("HTTP Sent = %lu\n", TotalSent);
    DBGLN("PDF Transfer Completed");
    TotalSent = 0;
    DownloadBusy = false;
    return true;
}
