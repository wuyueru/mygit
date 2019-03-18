#include "tcp.h"
#include "led.h"

extern char *TAG;


extern const uint8_t iii_html_start[] asm("_binary_iii_html_start");
extern const uint8_t iii_html_end[]   asm("_binary_iii_html_end");

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

 void tcp_recv_task(void *pvParameters)
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
            // 提取有效字符段
            check_string((char *)strbuffer,(char *)rx_buffer);
            ESP_LOGI(TAG, "strbuffer:%s", strbuffer);
            /**************led*****************/
            if(strcmp(strbuffer,"LED_ON") == 0)
            {
               led_on();
            }
            else if(strcmp(strbuffer,"LED_OFF") == 0)
            {
               led_off();
            }
            len = sprintf(sbuf,"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: %d\r\n\r\n",filelen);

            memcpy(sbuf + len, iii_html_start,filelen);

            send(sock, sbuf, len+filelen, 0);
            ESP_LOGI(TAG, "len: %d",len);
  
       

        }
    }
     close(sock);
    //free(filebuf);
    free(sbuf);
    vTaskDelete(NULL);
           
}



void tcp_server_task(void *pvParameters)
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

