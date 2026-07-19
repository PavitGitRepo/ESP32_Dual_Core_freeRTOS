#ifndef __UART_MANAGER_H__
#define __UART_MANAGER_H__

#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"

#define CMD_START_REPORT      0x01
#define CMD_ACK_READY         0x02
#define CMD_REPORT_SIZE       0x03
#define CMD_ACK_SIZE          0x04
#define CMD_DATA              0x05
#define CMD_ACK_DATA          0x06
#define CMD_END_REPORT        0x07
#define CMD_ACK_END           0x08
#define CMD_ERROR             0x09

extern uint8_t RxBuffer[UART_BUFFER_SIZE];
extern volatile bool ReportFinished;
extern volatile uint32_t RxLength;
extern volatile bool BufferReady;

typedef enum
{
    UART_STATE_IDLE = 0,
    UART_STATE_REQUEST_DATA,
    UART_STATE_WAIT_READY,
    UART_STATE_WAIT_SIZE,
    UART_STATE_RECEIVING,
    UART_STATE_WAIT_DATA,
    UART_STATE_FINISHED,
    UART_STATE_ERROR
}UART_State_t;

typedef struct
{
    uint8_t  Command;
    uint16_t Length;
}UART_Header_t;

void UART_Manager_Init(void);
void UART_Manager_Task(void *parameter);

bool UART_RequestReport(void);
uint32_t UART_GetData(uint8_t *buffer,uint32_t maxLength);
bool UART_ReportAvailable(void);
bool UART_IsFinished(void);
bool UART_IsBusy(void);
uint32_t UART_GetReportSize(void);

uint32_t UART_GetReceivedBytes(void);
UART_State_t UART_GetState(void);

void UART_ProcessWaitReady(void);
void UART_ProcessWaitSize(void);
void UART_ProcessReceive(void);

bool UART_SendCommand(uint8_t cmd);
bool UART_SendHeader(uint8_t cmd,uint16_t length);
bool UART_SendACK(uint8_t ack);
bool UART_ReadBytes(uint8_t *buffer,uint32_t length);

bool UART_ReadHeader(UART_Header_t *header);
void UART_FlushRX(void);

void UART_Reset(void);

#endif