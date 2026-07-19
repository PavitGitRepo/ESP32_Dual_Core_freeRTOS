#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <Arduino.h>

bool HTTP_Server_Init(void);

void HTTP_Server_Task(void *parameter);

#endif