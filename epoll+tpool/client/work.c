#include "work.h"


extern char *mbegin;
extern int port;
int fileinfo_len = sizeof(struct fileinfo);
socklen_t sockaddr_len = sizeof(struct sockaddr);
int head_len = sizeof(struct head);
int freeid = 0;
struct conn gconn[CONN_MAX];
int conn_len = sizeof(struct conn);

int createfile(char *filename, int size)
{
	int fd = open(filename, O_RDWR | O_CREAT);
	fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	lseek(fd, size-1, SEEK_SET);
	write(fd, "", 1);
	close(fd);
	return 0;
}


struct head * new_fb_head(char *filename, int tempid, int *offset)
{
    struct head * p_fhead = (struct head *)malloc(head_len);
    bzero(p_fhead, head_len);
    strcpy(p_fhead->filename, filename);
    p_fhead->id = tempid;
    p_fhead->offset = *offset;
    p_fhead->bs = BLOCKSIZE;
    *offset += BLOCKSIZE;
    return p_fhead;
}


void send_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, int *p_last_bs)
{	

    bzero(p_finfo, fileinfo_len);
    strcpy(p_finfo->filename, fname);
    p_finfo->filesize = p_fstat->st_size;

    int count = (p_fstat->st_size) / BLOCKSIZE;
    if(p_fstat->st_size%BLOCKSIZE == 0)
    {
        p_finfo->count = count;
    }
    else
    {
        p_finfo->count = count+1;
        *p_last_bs = p_fstat->st_size - BLOCKSIZE*count;
    }
    p_finfo->bs = BLOCKSIZE;

    char send_buf[100]= {0};
    memcpy(send_buf,p_finfo,fileinfo_len);
    send(sock_fd,send_buf,fileinfo_len, 0);
}

void send_filedata(struct head * p_fhead,int sockfd)
{
    
    struct send_pack temp;
    int count = 0;

    memset(&temp,0,sizeof(temp));
    temp.send_cmd = FILE_DATA;
    send_pk(&temp,sockfd);
    
    
    usleep(5);
    send(sockfd,p_fhead,sizeof(*p_fhead),0);
    
    int i = 0;

    int num = p_fhead->bs/SEND_SIZE;
    int remain_size = p_fhead->bs%SEND_SIZE;
    char *fp = mbegin + p_fhead->offset;
    
    for(i = 0;i < num;i++)
    {
        usleep(45);
        send(sockfd,fp,SEND_SIZE,0);
        fp = fp + SEND_SIZE;
    }
    if(remain_size != 0)
    {
        usleep(45);
        send(sockfd,fp,remain_size,0);
    }

    free(p_fhead);
    
}


int Client_init(char *ip)
{
    //创建socket
    int sock_fd = socket(AF_INET,SOCK_STREAM, 0);

    //构建地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    //连接服务器
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sockaddr_len) < 0)
    {
        perror("connect");
        exit(-1);
    }
    return sock_fd;
}


void set_fd_noblock(int fd)
{
    int flag=fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	return;
}

void sendfile(int info_fd)
{
    extern int flag;
    struct send_pack sdpk;
    extern struct current_data temp_data0;
    int cmd = 0;
    flag = 0;
    int tempid = 0;
    char filename[MAX_SIZE] = {0};
    int fd = 0;

    memset(&sdpk,0,sizeof(sdpk));
    
    printf("> Please input filename : ");
    
    scanf("%s",filename);
    
    sdpk.send_cmd = FILE_INFO;
    memcpy(sdpk.send_msg1,filename,strlen(filename));
    
    memcpy(sdpk.sourse_name,temp_data0.current_name,strlen(temp_data0.current_name));
    printf("1 >> Single user\n");
    printf("2 >> Every user in a group\n");
    printf("> Please input your choice :\n");
    
    scanf("%d",&cmd);
    
    system("clear");

    printf("> Please input the name you want to send to : ");
    scanf("%s",sdpk.send_msg0);
    sdpk.result = cmd;

    
    send_pk(&sdpk,info_fd);
    
    if((fd = open(filename, O_RDWR)) == -1 )
    {
        printf("open erro ！\n");
        exit(-1);
    }   

    struct stat filestat;
    fstat(fd ,&filestat);
    int last_bs=0;
    struct fileinfo finfo;
    send_fileinfo(info_fd, filename, &filestat, &finfo, &last_bs);
    
    recv(info_fd,&tempid,4,0);
    printf("tempid = %d\n",tempid);
    //usleep(15);
    mbegin = (char *)mmap(NULL, filestat.st_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd , 0);
    close(fd);
 
    int j=0, num=finfo.count, offset=0;

    struct head * p_fhead;
    int head_len = sizeof(struct head);
    if(last_bs == 0)
    {
        for(j=0; j<num; j++)
        {
            p_fhead = new_fb_head(filename, tempid, &offset);
            send_filedata(p_fhead,info_fd);
        }
    }

    else
    {
        for(j=0; j<num-1; j++)
        {
            p_fhead = new_fb_head(filename,tempid, &offset);
            send_filedata(p_fhead,info_fd);
        }
        
        struct head * p_fhead = (struct head *)malloc(head_len);
        bzero(p_fhead, head_len);
        strcpy(p_fhead->filename, filename);
        p_fhead->id = tempid;
        p_fhead->offset = offset;
        p_fhead->bs = last_bs;
        send_filedata(p_fhead,info_fd);
    }
    flag = 1;
}


int recv_fileinfo(int sockfd)
{
    
    struct fileinfo finfo;
    memset(&finfo,0,sizeof(finfo));
    recv(sockfd,&finfo,sizeof(finfo),0);
    
    char filepath[100] = {0};
    printf("%s\n",finfo.filename);
    sprintf(filepath,"%s",finfo.filename);
    createfile(filepath,finfo.filesize);
    
    int fd=0;
    if((fd = open(filepath, O_RDWR)) == -1 )
    {
        printf("open file erro\n");
        exit(-1);
    }

    char *map = (char *)mmap(NULL, finfo.filesize, PROT_WRITE|PROT_READ, MAP_SHARED, fd , 0);

    close(fd);

    while( gconn[freeid].used )
    {
        ++freeid;
        freeid = freeid%CONN_MAX;
    }

    bzero(&gconn[freeid].filename, FILENAME_MAXLEN);
    gconn[freeid].info_fd = sockfd;
    strcpy(gconn[freeid].filename, finfo.filename);
    gconn[freeid].filesize = finfo.filesize;
    gconn[freeid].count = finfo.count;
    gconn[freeid].bs = finfo.bs;
    gconn[freeid].mbegin = map;
    gconn[freeid].recvcount = 0;
    gconn[freeid].used = 1;

    char freeid_buf[INT_SIZE]={0};
    memcpy(freeid_buf, &freeid, INT_SIZE);
    usleep(5);
    send(sockfd, freeid_buf, INT_SIZE, 0);

    return finfo.count;
}


void recv_filedata(int sockfd)
{
    //int a = 0;
    //recv(sockfd,&a,4,0);
    //printf("a = %d\n",a);

    //printf("start\n");
    struct head fhead;
    memset(&fhead,0,sizeof(fhead));
    recv(sockfd,&fhead,sizeof(fhead),0);

    //printf("%d\n",fhead.offset);
    //printf("%d\n",fhead.bs);
    
    int recv_id = fhead.id;
    
    int recv_offset = fhead.offset;
    char *fp = gconn[recv_id].mbegin + recv_offset;

    
    int i;
    int total_size = fhead.bs;    
    int remain_size = 0;
    remain_size = total_size % RECVBUF_SIZE;
    int num = 0;
    num = total_size/RECVBUF_SIZE;    

    for(i = 0;i < num;i++)
    {
        recv(sockfd, fp, RECVBUF_SIZE,0);
        fp = fp + RECVBUF_SIZE;
    }
    if(remain_size != 0)
    {
        recv(sockfd,fp,remain_size,0);
    }
    //printf("aa:");
    //printf("%d,%d\n",gconn[recv_id].recvcount,gconn[recv_id].count);
    (gconn[recv_id].recvcount)++;
    if(gconn[recv_id].recvcount == gconn[recv_id].count)
    {
        printf("Recive success\n");
        munmap((void *)gconn[recv_id].mbegin, gconn[recv_id].filesize);
        //printf("Recive success\n");
        bzero(&gconn[recv_id], conn_len);
    }
}
void recvfile(int sockfd)
{
    struct send_pack sdpk;
    extern struct current_data temp_data0;
    extern int flag;
    int count = 0;
    int i = 0;

    memset(&sdpk,0,sizeof(sdpk));
    sdpk.send_cmd = SHOW;
    memcpy(sdpk.sourse_name,temp_data0.current_name,strlen(temp_data0.current_name));
    sdpk.send_id1 = temp_data0.current_id1;
    sdpk.result = 5;
    send_pk(&sdpk,sockfd);
    memset(&sdpk,0,sizeof(sdpk));
    sdpk.send_cmd = RECV_FILE;
    printf("> Please input the id of the file you want to recive : ");
    scanf("%d",&(sdpk.send_id2));
    memcpy(sdpk.sourse_name,temp_data0.current_name,strlen(temp_data0.current_name));
    sdpk.send_id1 = temp_data0.current_id1;
    
    flag = 0;
    send_pk(&sdpk,sockfd);
   
    count = recv_fileinfo(sockfd);
    
    for(i = 0;i < count;i++)
    {
        //printf("i = %d\n",i);
        recv_filedata(sockfd);

    }
    //press_anykey(0,0);
    flag = 1;
}

void udp_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, int *p_last_bs,struct sockaddr_in *addr,int *len)
{   

    //bzero(p_finfo, fileinfo_len);
    memset(p_finfo,0,sizeof(*p_finfo));
    strcpy(p_finfo->filename, fname);
    p_finfo->filesize = p_fstat->st_size;

    int count = (p_fstat->st_size) / BLOCKSIZE;
    if(p_fstat->st_size%BLOCKSIZE == 0)
    {
        p_finfo->count = count;
    }
    else
    {
        p_finfo->count = count+1;
        *p_last_bs = p_fstat->st_size - BLOCKSIZE*count;
    }
    p_finfo->bs = BLOCKSIZE;
    sendto(sock_fd,p_finfo,sizeof(*p_finfo),0,(struct sockaddr *)addr,*len);   
}

void udp_filedata(struct head * p_fhead,int sockfd,struct sockaddr_in *addr,int *len)
{
    
    sendto(sockfd,p_fhead,sizeof(*p_fhead),0,(struct sockaddr *)addr,*len);
    
    int i = 0;
    int send_size = 0;
    int num = p_fhead->bs/SEND_SIZE;
    int remain_size = p_fhead->bs%SEND_SIZE;
    char *fp = mbegin + p_fhead->offset;
    
    for(i = 0;i < num;i++)
    {
        usleep(20);
        sendto(sockfd,fp,SEND_SIZE,0,(struct sockaddr *)addr,*len);
        //printf("send_size = %d\n",send_size);
        fp = fp + SEND_SIZE;
    }
    if(remain_size != 0)
    {
        usleep(20);
        send_size = sendto(sockfd,fp,remain_size,0,(struct sockaddr *)addr,*len);
        //printf("send_size = %d\n",send_size);
    }

    free(p_fhead);
    
}

void udpfile(int info_fd)
{


    int udpfd = socket(AF_INET,SOCK_DGRAM,0);

    struct sockaddr_in addr;

    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6677);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t len = sizeof(addr);
    //建立udp连接   
    
    extern int flag;
    struct send_pack sdpk;
    extern struct current_data temp_data0;
    int cmd = 0;
    flag = 0;
    int tempid = 0;
    char filename[MAX_SIZE] = {0};
    int fd = 0;

    memset(&sdpk,0,sizeof(sdpk));
    
    printf("> Please input filename : ");
    
    scanf("%s",filename);
    
    sdpk.send_cmd = FILE_INFO;
    memcpy(sdpk.send_msg1,filename,strlen(filename));
    
    memcpy(sdpk.sourse_name,temp_data0.current_name,strlen(temp_data0.current_name));
    printf("1 >> Single user\n");
    printf("2 >> Every user in a group\n");
    printf("> Please input your choice :\n");
    
    scanf("%d",&cmd);
    
    system("clear");

    printf("> Please input the name you want to send to : ");
    scanf("%s",sdpk.send_msg0);
    sdpk.result = cmd;

    
    send_pk(&sdpk,info_fd);
    //以tcp的方式，发送基础文件信息
    if((fd = open(filename, O_RDWR)) == -1 )
    {
        printf("open erro ！\n");
        exit(-1);
    }   

    struct stat filestat;
    fstat(fd ,&filestat);
    int last_bs=0;
    struct fileinfo finfo;
    udp_fileinfo(udpfd, filename, &filestat, &finfo, &last_bs,&addr,&len);
    #if 1
    //recv(udpfd,&tempid,4,0);

    mbegin = (char *)mmap(NULL, filestat.st_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd , 0);
    close(fd);
 
    int j=0, num=finfo.count, offset=0;

    struct head * p_fhead;
    int head_len = sizeof(struct head);
    if(last_bs == 0)
    {
        for(j=0; j<num; j++)
        {
            p_fhead = new_fb_head(filename, tempid, &offset);
            udp_filedata(p_fhead,udpfd,&addr,&len);
        }
    }

    else
    {
        for(j=0; j<num-1; j++)
        {
            p_fhead = new_fb_head(filename,tempid, &offset);
            udp_filedata(p_fhead,udpfd,&addr,&len);
        }
        
        struct head * p_fhead = (struct head *)malloc(head_len);
        bzero(p_fhead, head_len);
        strcpy(p_fhead->filename, filename);
        p_fhead->id = tempid;
        p_fhead->offset = offset;
        p_fhead->bs = last_bs;
        udp_filedata(p_fhead,udpfd,&addr,&len);
    }
    close(udpfd);
    press_anykey(0,0);
    flag = 1;
    #endif
   
}
