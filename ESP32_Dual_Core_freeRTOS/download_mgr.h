/**********************************************************************
 *
 * File        : download_mgr.h
 *
 *********************************************************************/

#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

void Download_Init(void);
bool Download_Start(WiFiClient &client);
void Download_Stop(void);
bool Download_IsBusy(void);

#endif