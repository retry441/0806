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

    int num = p_fhead->bs/SEND_SIZE;
    int remain_size = p_fhead->bs%SEND_SIZE;
    char *fp = mbegin + p_fhead->offset;
    
    for(i = 0;i < num;i++)
    {
        //usleep(5);
        sendto(sockfd,fp,SEND_SIZE,0,(struct sockaddr *)addr,*len);
        fp = fp + SEND_SIZE;
    }
    if(remain_size != 0)
    {
        //usleep(5);
        send(sockfd,fp,remain_size,0,(struct sockaddr *)addr,*len);
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
    send_fileinfo(udpfd, filename, &filestat, &finfo, &last_bs,&addr,&len);
    #if 0
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
            //send_filedata(p_fhead,udpfd);
        }
    }

    else
    {
        for(j=0; j<num-1; j++)
        {
            p_fhead = new_fb_head(filename,tempid, &offset);
            //send_filedata(p_fhead,udpfd);
        }
        
        struct head * p_fhead = (struct head *)malloc(head_len);
        bzero(p_fhead, head_len);
        strcpy(p_fhead->filename, filename);
        p_fhead->id = tempid;
        p_fhead->offset = offset;
        p_fhead->bs = last_bs;
        //send_filedata(p_fhead,udpfd);
    }
    close(udpfd);
    flag = 1;
    #endif
   
}