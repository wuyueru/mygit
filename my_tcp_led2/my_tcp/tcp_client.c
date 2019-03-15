

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <sys/param.h>

#define HOST_IP_ADDR "192.168.1.105"
#define PORT 3333


void main()
{
    char str[50] = {0};
    char recvbuf[50] = {0};
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(&addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    addr.sin_port = htons(PORT);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0)
    {
        printf("socket is error\r\n");
        return;
    }

    int err = connect(sockfd,(struct sockaddr *) &addr, sizeof(addr));
    if(err < 0)
    {
        printf("connect is error\r\n");
        return;
    }
    printf("connect is success\r\n");

    while(1)
    {
	memset(str,0,sizeof(str));
	scanf("%s",str);
        err = send(sockfd,str,sizeof(str),0);
        if(err < 0)
        {
            printf("send is error\r\n");
            return;
        }
        printf("send is success\r\n");

        err = recv(sockfd,recvbuf,sizeof(recvbuf),0);
        if(err < 0)
        {
            printf("recv is error\r\n");
            return;
        }
        printf("host recv %s\r\n",recvbuf);

    }

}
