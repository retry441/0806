#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_SIZE 128
int main()
{



    int udpfd = socket(AF_INET,SOCK_DGRAM,0);

    struct sockaddr_in addr;

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6677);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    char buf[MAX_SIZE] = "hello";
    int n = 0;
    socklen_t len = sizeof(addr);
    while(n < 5)
    {    
        sendto(udpfd,&buf,sizeof(buf),0,(struct sockaddr *)&addr,len);
        n++;
        sleep(1);
    } 
    close(udpfd);
}