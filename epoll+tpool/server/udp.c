#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mman.h>

#define FILENAME_MAXLEN 30
#define MAX_SIZE 128
#define SEND_SIZE    8192  

struct fileinfo
{
    char filename[FILENAME_MAXLEN];     //文件名
    int filesize;                       //文件大小
    int count;                          //分块数量
    int bs;                             //标准分块大小
};

struct conn
{
    int info_fd;                      //信息交换socket：接收文件信息、文件传送通知client
    char filename[FILENAME_MAXLEN];   //文件名
    int filesize;                     //文件大小
    int bs;                           //分块大小
    int count;                        //分块数量
    int recvcount;                    //已接收块数量，recv_count == count表示传输完毕
    char *mbegin;                     //mmap起始地址
    int used;                         //使用标记，1代表使用，0代表可用
};

struct head
{
    char filename[FILENAME_MAXLEN];     //文件名
    int id;                             //分块所属文件的id，gconn[CONN_MAX]数组的下标
    int offset;                         //分块在原文件中偏移
    int bs;                             //本文件块实际大小
};

int createfile(char *filename, int size)
{
	int fd = open(filename, O_RDWR | O_CREAT);
	fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	lseek(fd,size - 1,SEEK_SET);
	write(fd,"",1);
	close(fd);
	return 0;
}
struct conn tempdata;

int main()
{
	printf("# Udp file server is running\n");
	int sockfd = socket(AF_INET,SOCK_DGRAM,0);
	int opt = 1;
	int i = 0;

	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6677);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(sockfd,(struct sockaddr *)&addr,sizeof(addr)) < 0)
	{
		perror("bind");
	}

	struct sockaddr_in cli;
	struct fileinfo info;
	socklen_t len = sizeof(cli);
	while(1)
	{
		memset(&info,0,sizeof(info));
		recvfrom(sockfd,&info,sizeof(info),0,(struct sockaddr *)&addr,&len);
		//sprintf(filepath,"%d_%s",temp_id,finfo.filename);

    	createfile(info.filename,info.filesize);
    
    	int fd=0;
    	if((fd = open(info.filename,O_RDWR)) == -1 )
		{
			printf("open file erro\n");
			exit(-1);
		}

    	char *map = (char *)mmap(NULL,info.filesize, PROT_WRITE|PROT_READ, MAP_SHARED, fd , 0);
    	close(fd);

    	memset(&tempdata,0,sizeof(tempdata));
    
    	tempdata.info_fd = sockfd;
    	strcpy(tempdata.filename, info.filename);
    	tempdata.filesize = info.filesize;
    	tempdata.count = info.count;
    	tempdata.bs = info.bs;
    	tempdata.mbegin = map;
    	tempdata.recvcount = 0;
    	tempdata.used = 1;
    	struct head fhead;
    	int recv_id = 0;
    	int recv_offset = 0;
    	int size = 0;                 
    	int count = 0;
    	int a = 0;
    	int j = 0;
    	char *fp = NULL;
    	int num = 0;
    	int remain_size = 0;
    	int recv_size = 0;
    	for(i = 0;i < info.count;i++)
    	{
    		memset(&fhead,0,sizeof(fhead));
    		recvfrom(sockfd,&fhead,sizeof(fhead),0,(struct sockaddr *)&addr,&len);
    		printf("offset = %d\n",fhead.offset);
    		recv_id = fhead.id;
    		recv_offset = fhead.offset;
    		fp = tempdata.mbegin + recv_offset;
    
    		num = fhead.bs/SEND_SIZE;
    		remain_size = fhead.bs%SEND_SIZE;
    
    
    		for(i = 0;i < num;i++)
    		{
        		recv_size = recvfrom(sockfd,fp,SEND_SIZE,0,(struct sockaddr *)&addr,&len);
        		//printf("recv_size = %d\n",recv_size);
        		fp = fp + SEND_SIZE;
    		}
    		if(remain_size != 0)
    		{
        		recv_size = recvfrom(sockfd,fp,remain_size,0,(struct sockaddr *)&addr,&len);
        		//printf("recv_size = %d\n",recv_size);
    		}

    		tempdata.recvcount++;
    		if(tempdata.recvcount == tempdata.count)
    		{
        		munmap((void *)tempdata.mbegin,tempdata.filesize);
        		printf("Recive success\n");
    		}
    	}

	}
		

	close(sockfd);
	return 0;
}