/***********************************************************************
 *
 * File : HTTP_Server.cpp
 *
 ***********************************************************************/
#include "HTTP_Server.h"
#include "Config.h"
#include "WebUI.h"
#include "Download_Manager.h"

static WiFiServer HttpServer(HTTP_PORT);
static bool HTTP_ReadRequest(WiFiClient &client,char *buffer,uint32_t length);
static void HTTP_HandleClient(WiFiClient &client);
static void HTTP_SendHeader(WiFiClient &client,const char *type);
static void HTTP_SendWebPage(WiFiClient &client);
static void HTTP_SendProgress(WiFiClient &client);
static void HTTP_SendNotFound(WiFiClient &client);

bool HTTP_Server_Init(void)
{
    WiFi.mode(WIFI_AP);
    if(!WiFi.softAP(WIFI_SSID,WIFI_PASSWORD))
    {
        DBGLN("Failed to start WiFi AP");
        return false;
    }
    HttpServer.begin();
    DBGLN("--------------------------------");
    DBGLN("HTTP Server Started");
    DBGF("IP : %s\n",WiFi.softAPIP().toString().c_str());
    DBGLN("--------------------------------");
    return true;
}


void HTTP_Server_Task(void *parameter)
{
    while(true)
    {
        WiFiClient client = HttpServer.available();
        if(client)
        {
            HTTP_HandleClient(client);
            client.stop();
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static bool HTTP_ReadRequest(WiFiClient &client,char *buffer,uint32_t length)
{
    uint32_t index = 0;
    uint32_t timeout = millis();
    while(client.connected())
    {
        while(client.available())
        {
            char c = client.read();
            if(c == '\n')
            {
                buffer[index] = 0;
                return true;
            }
            if(c == '\r')
                continue;

            if(index < (length - 1))
            {
                buffer[index++] = c;
            }
        }
        if((millis() - timeout) > 3000)
            return false;

        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return false;
}

static void HTTP_SendHeader(WiFiClient &client,const char *type)
{
    client.println("HTTP/1.1 200 OK");
    client.print("Content-Type: ");
    client.println(type);
    client.println("Connection: close");
    client.println();
}

static void HTTP_HandleClient(WiFiClient &client)
{
    char request[HTTP_BUFFER_SIZE];
    memset(request, 0, sizeof(request));

    if(!HTTP_ReadRequest(client,request,sizeof(request)))
    {
        return;
    }

    DBGF("HTTP Request : %s\n", request);
    if(strncmp(request,"GET / ",6) == 0)
    {
        HTTP_SendWebPage(client);
        return;
    }

    if(strncmp(request,"GET /download",13) == 0)
    {
        uint32_t t0 = millis();
        xTaskNotifyGive(UART_TaskHandle);
        DBGF("Download Request: %ld\n", t0);
        Download_Start(client);
        return;
    }

    HTTP_SendNotFound(client);
}

static void HTTP_SendWebPage(WiFiClient &client)
{
    HTTP_SendHeader(client,"text/html");
    client.print(WEB_PAGE);
}

static void HTTP_SendNotFound(WiFiClient &client)
{
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<html>");
    client.println("<body>");
    client.println("<h2>404 - Page Not Found</h2>");
    client.println("</body>");
    client.println("</html>");
}
