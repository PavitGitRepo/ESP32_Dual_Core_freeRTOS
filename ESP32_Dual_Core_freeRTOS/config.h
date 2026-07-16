#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>

// WiFi Configuration

#define WIFI_SSID           "ESP32_PDF_Server"
#define WIFI_PASSWORD       "12345678"

#define HTTP_PORT           80

// UART Configuration

#define UART_PORT           Serial2

#define UART_BAUDRATE       921600

#define UART_TX_PIN         17
#define UART_RX_PIN         16

// Buffers

#define UART_BUFFER_SIZE        4096
#define HTTP_BUFFER_SIZE        512
#define JSON_BUFFER_SIZE        256

// Download

#define DOWNLOAD_FILENAME       "Report.pdf"

#define STM32_PACKET_TIMEOUT    3000

// Progress

#define PROGRESS_UPDATE_INTERVAL    200

// Debug

#define ENABLE_DEBUG    1

#if ENABLE_DEBUG

#define DBGF(...)       Serial.printf(__VA_ARGS__)
#define DBGLN(x)        Serial.println(x)

#else

#define DBGF(...)
#define DBGLN(x)

#endif

// Download State
typedef struct
{
    uint8_t Data[UART_BUFFER_SIZE];
    uint32_t Length;
} UART_Packet_t;

extern TaskHandle_t HTTP_TaskHandle;
extern TaskHandle_t UART_TaskHandle;
extern QueueHandle_t data_queue;

typedef enum
{
    STATE_IDLE = 0,

    STATE_WAITING_FOR_STM32,

    STATE_RECEIVING,

    STATE_FINISHED,

    STATE_TIMEOUT,

    STATE_ERROR

} DownloadState_t;

// Download Status

typedef struct
{
    DownloadState_t State;

    bool ClientConnected;

    bool STM32Connected;

    bool DownloadFinished;

    uint32_t TotalBytes;

    uint32_t BytesReceived;

    uint32_t BytesSent;

    uint32_t TransferSpeed;

    uint8_t Progress;

} DownloadStatus_t;

// Global Status

extern DownloadStatus_t DownloadStatus;
extern TaskHandle_t HTTP_TaskHandle;
extern TaskHandle_t UART_TaskHandle;

// Global UART Buffer

extern uint8_t UART_Buffer[UART_BUFFER_SIZE];

#endif