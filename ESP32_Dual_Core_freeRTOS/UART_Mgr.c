#include "UART_Manager.h"
#include "Config.h"

static UART_State_t UART_State = UART_STATE_IDLE;
static bool ReportBusy = false;

static uint32_t ReportSize = 0;
static uint32_t BytesReceived = 0;
static uint32_t LastReceiveTime = 0;
static uint32_t TotalTime = 0;
uint8_t RxBuffer[UART_BUFFER_SIZE];

volatile uint32_t RxLength = 0;
volatile bool ReportFinished = false;
volatile bool BufferReady = false;

static uint32_t PacketLength = 0;
static uint32_t PacketIndex = 0;

static bool UART_ErrorFlag = false;

void UART_Manager_Init(void)
{
    UART_PORT.setRxBufferSize(UART_BUFFER_SIZE);
    UART_PORT.begin(UART_BAUDRATE,SERIAL_8N1,UART_RX_PIN,UART_TX_PIN); 
    UART_Reset();
    DBGLN("UART Manager Initialized");
}

void UART_Reset(void)
{
    ReportBusy = false;
    ReportFinished = false;
    ReportSize = 0;
    BytesReceived = 0;
    RxLength = 0;
    UART_State = UART_STATE_IDLE;
    while(UART_PORT.available())
    {
        UART_PORT.read();
    }
}

bool UART_RequestReport(void)
{
    if(ReportBusy)
        return false;
    UART_Reset();
    ReportBusy = true;
    UART_State = UART_STATE_WAIT_READY;
    LastReceiveTime = millis();
    DBGLN("Sending START_REPORT");
    UART_PORT.write(CMD_START_REPORT);
    return true;
}

bool UART_IsBusy(void)
{
    return ReportBusy;
}

bool UART_IsFinished(void)
{
    return ReportFinished;
}

UART_State_t UART_GetState(void)
{
    return UART_State;
}

uint32_t UART_GetReportSize(void)
{
    return ReportSize;
}

uint32_t UART_GetReceivedBytes(void)
{
    return BytesReceived;
}

bool UART_ReportAvailable(void)
{
    return (RxLength > 0);
}

uint32_t UART_GetData(uint8_t *buffer,uint32_t maxLength)
{
    if(RxLength == 0)
        return 0;

    uint32_t len = RxLength;
    if(len > maxLength)
        len = maxLength;

    memcpy(buffer,RxBuffer,len);
    return len;
}

void UART_Manager_Task(void *parameter)
{
    for(;;)
    { 
        while(1)  
        { 
            switch(UART_State)
            {
                case UART_STATE_IDLE:
                    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
                    UART_State = UART_STATE_REQUEST_DATA;
                break;

                case UART_STATE_REQUEST_DATA:
                    if(!UART_RequestReport())
                    {
                        UART_State = UART_STATE_ERROR;
                    }
                break;

                case UART_STATE_WAIT_READY:
                    UART_ProcessWaitReady();
                break;

                case UART_STATE_WAIT_SIZE:
                    UART_ProcessWaitSize();
                break;

                case UART_STATE_RECEIVING:
                    UART_ProcessReceive();
                    break;

                case UART_STATE_WAIT_DATA:
                    break;

                case UART_STATE_FINISHED:
                    UART_State = UART_STATE_IDLE;
                    break;

                case UART_STATE_ERROR:
                    UART_ErrorFlag = true;                    
                    break;

                default:
                    UART_State = UART_STATE_IDLE;
                    break;
            }
            if(UART_ErrorFlag)
            {
              UART_State = UART_STATE_IDLE;  
            }
        }
    }
}

void UART_ProcessWaitReady(void)
{
    if (UART_PORT.available() == 0)
        return;

    uint8_t cmd = UART_PORT.read();

    if (cmd == CMD_ACK_READY)
    {
        DBGLN("ACK_READY Received");
        UART_State = UART_STATE_WAIT_SIZE;
        LastReceiveTime = millis();
    }
    else
    {
        DBGLN("Unexpected Command");
        UART_State = UART_STATE_ERROR;
    }
}

void UART_ProcessWaitSize(void)
{
    static uint8_t Buffer[4];
    static uint8_t Index = 0;

    if (Index == 0)
    {
        if (UART_PORT.available() == 0)
            return;
    }

    while (UART_PORT.available() && Index < 4)
    {
        Buffer[Index++] = UART_PORT.read();
    }

    if (Index < 4)
        return;

    memcpy(&ReportSize, Buffer, 4);
    DBGF("Report Size = ");DBGLN(ReportSize);
    UART_SendCommand(CMD_ACK_SIZE);

    Index = 0;
    BytesReceived = 0;
    UART_State = UART_STATE_RECEIVING;
    LastReceiveTime = millis();
}


void UART_ProcessReceive(void)
{
    while(BytesReceived < ReportSize)
    {
        int avail = UART_PORT.available();

        if(avail > 0)
        {
            if(avail > (UART_BUFFER_SIZE - RxLength))
                avail = UART_BUFFER_SIZE - RxLength;

            int n = UART_PORT.readBytes((char *)&RxBuffer[RxLength], avail);
            LastReceiveTime = millis();

            RxLength += n;
            BytesReceived +=n;
        }

        if(RxLength == UART_BUFFER_SIZE)
        {
            UART_Packet_t DataPacket;
            memcpy(DataPacket.Data, RxBuffer, RxLength);
            DataPacket.Length = RxLength;
            if(xQueueSend(data_queue, &DataPacket, portMAX_DELAY) == pdTRUE)
                RxLength = 0;

            vTaskDelay(1);
        }
    }

    if(RxLength)
    {
        UART_Packet_t DataPacket;
        memcpy(DataPacket.Data, RxBuffer, RxLength);
        DataPacket.Length = RxLength;
        xQueueSend(data_queue, &DataPacket, portMAX_DELAY);
        RxLength = 0;
    }

    UART_SendCommand(CMD_ACK_END);
    ReportFinished = true;
    ReportBusy = false;
    UART_State = UART_STATE_FINISHED;
    DBGF("UART BytesReceived %d\n", BytesReceived);

    if((millis() - LastReceiveTime) > STM32_PACKET_TIMEOUT)
    {
        DBGLN("UART Timeout");
        DBGF("RxLength %d, BytesReceived %d", RxLength, BytesReceived); 
        UART_State = UART_STATE_ERROR;
        ReportBusy = false;
    }
}

/*
void UART_ProcessReceive(void)
{
    UART_Packet_t DataPacket;
    static uint16_t Pcount = 0;
    while(UART_PORT.available())
    {
        uint8_t data = UART_PORT.read();
        LastReceiveTime = millis();

        if(RxLength < UART_BUFFER_SIZE)
        {
            RxBuffer[RxLength++] = data;
        }
        BytesReceived++;

        if(BytesReceived >= ReportSize)
        {
            DBGF("Report Completed %d\n", BytesReceived);
            UART_SendCommand(CMD_ACK_END);

            ReportFinished = true;
            ReportBusy = false;
            UART_State = UART_STATE_FINISHED;
            //return;
        }
    }

    if(RxLength != 0)
    {
        DataPacket.Data = RxBuffer;
        DataPacket.Length = RxLength;
        if(xQueueSend(data_queue, (void *)&DataPacket, 10) == pdTRUE)
        {
            RxLength = 0;
            UART_SendCommand(CMD_ACK_DATA);
            return;
        }
    }

    if((millis() - LastReceiveTime) > STM32_PACKET_TIMEOUT)
    {
        DBGLN("UART Timeout");
        DBGF("RxLength %d, BytesReceived %d", RxLength, BytesReceived); 
        UART_State = UART_STATE_ERROR;
        ReportBusy = false;
    }
}
*/
bool UART_SendCommand(uint8_t cmd)
{
    UART_PORT.write(cmd);
    UART_PORT.flush();
    return true;
}

bool UART_ReadBytes(uint8_t *buffer,
                    uint32_t length)
{
    uint32_t index = 0;

    while(index < length)
    {
        if(UART_PORT.available())
            buffer[index++] = UART_PORT.read();

        if((millis() - LastReceiveTime) > STM32_PACKET_TIMEOUT)
            return false;
    }
    return true;
}