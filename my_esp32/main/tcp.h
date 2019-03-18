#ifndef _TCP_H_
#define _TCP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "esp_log.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PORT 80

void check_string(char *str,char *rx);
 void tcp_recv_task(void *pvParameters);
void tcp_server_task(void *pvParameters);

#endif
