/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "driver/gpio.h"

#include <sys/param.h>

#include "lwip/sockets.h"
#include <lwip/netdb.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>



/* The examples use WiFi configuration that you can set via 'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#define PORT 80

#define BLINK_GPIO 22

 

 char rx_buffer[200] = {0};
 char strbuffer[128] = {0};
 char addr_str[128];
 int addr_family;
 int ip_protocol;

 struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6

 int sock;
 int len;
 int sendflag = 0;
 int fd;



     
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event 
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "wifi station";

static int s_retry_num = 0;

extern const uint8_t iii_html_start[] asm("_binary_iii_html_start");
extern const uint8_t iii_html_end[]   asm("_binary_iii_html_end");

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
                esp_wifi_connect();
                xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                s_retry_num++;
                ESP_LOGI(TAG,"retry to connect to the AP");
            }
            ESP_LOGI(TAG,"connect to the AP fail\n");
            break;
        }
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_sta()
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void check_string(char *str,char *rx)
{
    int n = 0;
    while(*rx != '\n')
    {
        if((*rx == '/') && (n == 0))
        {
            n++;
            rx++;
            while(*rx != 'H')
            {
                *str = *rx;
                 str++;
                 rx++;
            }
            str--;
            *str = '\0';
        }
        rx++;
    }
}

static void tcp_send_task(void *pvParameters)
{
    int err;
    int len;
    int sock = *(int *)pvParameters;
    while(1)
    {
        if(sendflag == 1)
        {
            sendflag = 0;
            len = sprintf(rx_buffer,"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 12\r\n\r\nHello world\r\n");
            err = send(sock, rx_buffer, len, 0);
            if (err < 0) {
                 ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                break;
            }
        }
        
    }
      
}


static void tcp_recv_task(void *pvParameters)
{
    int sock = *(int *)pvParameters;
    int filelen = 0;
   // char filebuf[1000] = {0};
   // char sbuf[7000] = {0};
    filelen = iii_html_end - iii_html_start;
   // char *filebuf = NULL;
    char *sbuf = NULL;
    
    //filebuf = (char *)malloc(filelen+1);
    sbuf = (char *)malloc(filelen + 200);

    while(1)
    {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occured during receiving
        if (len < 0) {
            ESP_LOGE(TAG, "recv failed: errno %d", errno);
            break;
        }
        // Connection closed
        else if (len == 0) {
            ESP_LOGI(TAG, "Connection closed");
            break;
        }
        // Data received
        else {
            // Get the sender's ip address as string
            if (sourceAddr.sin6_family == PF_INET) {
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
            } else if (sourceAddr.sin6_family == PF_INET6) {
                inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
            }

            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
            ESP_LOGI(TAG, "%s", rx_buffer);
           check_string((char *)strbuffer,(char *)rx_buffer);
            ESP_LOGI(TAG, "strbuffer:%s", strbuffer);
            /**************led*****************/
            if(strcmp(strbuffer,"LED_ON") == 0)
            {
                gpio_set_level(BLINK_GPIO, 0);
            }
            else if(strcmp(strbuffer,"LED_OFF") == 0)
            {
                gpio_set_level(BLINK_GPIO, 1);
            }
          // memcpy(filebuf,iii_html_start,1000);
          //  memcpy(filebuf,iii_html_start,filelen);
          //  ESP_LOGI(TAG, "filebuf: %s",filebuf);
            len = sprintf(sbuf,"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: %d\r\n\r\n",filelen);

            memcpy(sbuf + len, iii_html_start,filelen);

            send(sock, sbuf, len+filelen, 0);
             ESP_LOGI(TAG, "len: %d",len);
        // ESP_LOGI(TAG, "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk");
       

        }
    }
     close(sock);
    //free(filebuf);
    free(sbuf);
    vTaskDelete(NULL);
           
}



static void tcp_server_task(void *pvParameters)
{
   // char rx_buffer[128] = {0};
   // char strbuffer[128] = {0};
   // char addr_str[128];
   // int addr_family;
   // int ip_protocol;

    while (1) {

#ifdef CONFIG_EXAMPLE_IPV4
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
#else // IPV6
        struct sockaddr_in6 destAddr;
        bzero(&destAddr.sin6_addr.un, sizeof(destAddr.sin6_addr.un));
        destAddr.sin6_family = AF_INET6;
        destAddr.sin6_port = htons(PORT);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(destAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket binded");

        err = listen(listen_sock, 5);
        if (err != 0) {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");

        //struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
        uint addrLen = sizeof(sourceAddr);
       
       int sockfd = 0;
        
        while (1) {

           sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
            if (sock < 0) {
                ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
                break;
             }
            ESP_LOGI(TAG, "Socket accepted");
            
            if(-1 != sock)
            {
                sockfd = sock;
                sendflag = 1;
                xTaskCreate(tcp_recv_task, "tcp_recv", 4096*2, &sockfd, 7, NULL);
                //xTaskCreate(tcp_send_task, "tcp_send", 4096, &sockfd, 8, NULL);
    
            }
            else {
                ESP_LOGE(TAG, "Shutting down socket and restarting...");
                shutdown(sock, 0);
                close(sock);
            }
            
        }

        
    }
    vTaskDelete(NULL);
}



void app_main()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

        gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BLINK_GPIO, 1);

    wifi_init_sta();

    xTaskCreate(tcp_server_task, "tcp_server", 4096*2, NULL, 5, NULL);
    
    
}
