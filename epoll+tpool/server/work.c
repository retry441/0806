#include "head.h"

int freeid = 0;
extern tpool_work_t *temp_work;
extern int epfd;
struct conn gconn[CONN_MAX];
//pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;
int fileinfo_len = sizeof(struct fileinfo);
socklen_t sockaddr_len = sizeof(struct sockaddr);
int head_len = sizeof(struct head);
int conn_len = sizeof(struct conn);

int recv_int(int conn_fd)
{
    char type_buf[INT_SIZE] = {0};
    char *p = type_buf;
    int recv_size = 0;
    while(1)
    {
        if( recv(conn_fd, p, 1, 0) == 1 )
        {
            ++recv_size;
            if(recv_size == INT_SIZE)
            {
                break;
            }
            ++p;
        }
    }

    int type = *((int*)type_buf);
    return type;
}

int createfile(char *filename, int size)
{
	int fd = open(filename, O_RDWR | O_CREAT);
	fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	lseek(fd,size - 1,SEEK_SET);
	write(fd,"",1);
	close(fd);
	return 0;
}

void *worker(void *argc)
{
    struct args *pw = (struct args *)argc;
    struct epoll_event ev;

    memset(&ev,0,sizeof(ev));
    int conn_fd = pw->fd;
    

    if(pw->handle1 != NULL)
    {
        pw->handle1(conn_fd,pw->sdpk);
        //pw->flag = NEED_IN;
    }
    ev.data.fd = conn_fd;
    ev.events = EPOLLIN|EPOLLONESHOT;
    epoll_ctl(epfd,EPOLL_CTL_MOD,conn_fd,&ev);
    return NULL;
}


void recv_fileinfo(int sockfd,struct send_pack recv_pack)
{
    extern sqlite3 *db;
    struct send_pack temp_data;
    char fileinfo_buf[100] = {0};
    int count = 0;
    int temp_id = 0;
    int n = 0;

    memset(&temp_data,0,sizeof(temp_data));
   
    

    
    struct fileinfo finfo;
    memset(&finfo,0,sizeof(finfo));
    recv(sockfd,&finfo,sizeof(finfo),0);
    

    char filepath[MAX_SIZE] = {0};
 
    count = search_data(db,"filelist",NULL,NULL,COUNT);
    temp_id = count + 1;
    temp_data.send_id1 = temp_id;
    memcpy(temp_data.sourse_name,recv_pack.send_msg1,strlen(recv_pack.send_msg1));

    memcpy(temp_data.send_msg0,recv_pack.sourse_name,strlen(recv_pack.sourse_name));
    switch(recv_pack.result)
    {
        case 1:
        {
            memcpy(temp_data.send_msg1,recv_pack.send_msg0,strlen(recv_pack.send_msg0));
            sprintf(temp_data.send_msg2,"null");
            break;
        }

        case 2:
        {
            memcpy(temp_data.send_msg2,recv_pack.send_msg0,strlen(recv_pack.send_msg0));
            sprintf(temp_data.send_msg1,"null");
            break;
        }
    }
    insert_data(db,"filelist",temp_data,7);
    #if 1
    sprintf(filepath,"%d_%s",temp_id,finfo.filename);
    createfile(filepath, finfo.filesize);
    
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
    memset(&gconn[freeid],0,sizeof(gconn[freeid]));
    
    gconn[freeid].info_fd = sockfd;
    strcpy(gconn[freeid].filename, finfo.filename);
    gconn[freeid].filesize = finfo.filesize;
    gconn[freeid].count = finfo.count;
    gconn[freeid].bs = finfo.bs;
    gconn[freeid].mbegin = map;
    gconn[freeid].recvcount = 0;
    gconn[freeid].used = 1;
    usleep(5);
    send(sockfd,&freeid,4,0);
    //printf("freeid = %d\n",freeid);
    #endif

}

/*接收文件块*/
void recv_filedata(int sockfd,struct send_pack recv_pack)
{
    
    struct head fhead;
    memset(&fhead,0,sizeof(fhead));
    recv(sockfd,&fhead,sizeof(fhead),0);
    int recv_id = fhead.id;

     
    int recv_offset = fhead.offset;
    char *fp = gconn[recv_id].mbegin + recv_offset;

    int size = 0;                 
    int count = 0;
    int a = 0;
    int i = 0;
    
    int num = fhead.bs/SEND_SIZE;
    int remain_size = fhead.bs%SEND_SIZE;
    
    
    for(i = 0;i < num;i++)
    {
        recv(sockfd,fp,SEND_SIZE,0);
        fp = fp + SEND_SIZE;
    }
    if(remain_size != 0)
    {
        recv(sockfd,fp,remain_size,0);
    }
    gconn[recv_id].recvcount++;
    if(gconn[recv_id].recvcount == gconn[recv_id].count)
    {
        munmap((void *)gconn[recv_id].mbegin, gconn[recv_id].filesize);

        printf("Recive success\n");

		memset(&gconn[recv_id],0,sizeof(gconn[recv_id]));
    }
}

/*初始化Server，监听Client*/
int Server_init(int port)
{
    int listen_fd;
    int opt = 1;

    struct sockaddr_in server_addr;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
        fprintf(stderr, "Creating server socket failed.");
        exit(-1);
    }
    set_fd_noblock(listen_fd);

    bzero(&server_addr, sockaddr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    if(bind(listen_fd, (struct sockaddr *) &server_addr, sockaddr_len) == -1)
    {
        fprintf(stderr, "Server bind failed.");
        exit(-1);
    }

    if(listen(listen_fd, LISTEN_QUEUE_LEN) == -1)
    {
        fprintf(stderr, "Server listen failed.");
        exit(-1);
    }
    return listen_fd;
}

void set_fd_noblock(int fd)
{
    int flag=fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
	return;
}



void init_args(int sockfd,struct send_pack recv,struct args *p_args)
{
    
    p_args->fd = sockfd;
    
    switch(recv.send_cmd)
    {
        case SIGNUP:
        {
            p_args->handle1 = handle_signup;
            break;
        }

        case LOGIN:
        {
            p_args->handle1 = handle_login;
            break;            
        }

        case FILE_INFO:
        {
            p_args->handle1 = recv_fileinfo;
            break;
        }

        case FILE_DATA:
        {
            p_args->handle1 = recv_filedata;
            break;
        }

        case REFINDA:
        {
            p_args->handle1 = handle_refindA;
            break;           
        }

        case REFINDB:
        {
            p_args->handle1 = handle_refindB;
            break;           
        }

        case OFFLINE:
        {
            p_args->handle1 = handle_offline;
            break;                 
        }

        case SHOW:
        {
            p_args->handle1 = handle_show;
            break; 
        }

        case MUTI_SEND:
        {
            p_args->handle1 = handle_send;
            break;            
        }

        case RECV_FILE:
        {
            p_args->handle1 = handle_recv;
            break;  
        }
    }
    
}

void copy_sendpack(struct send_pack *recv,struct send_pack send)
{
    recv->send_cmd = send.send_cmd;
    recv->result = send.result;
    memcpy(recv->sourse_name,send.sourse_name,strlen(send.sourse_name));
    memcpy(recv->send_msg0,send.send_msg0,strlen(send.send_msg0));
    memcpy(recv->send_msg1,send.send_msg1,strlen(send.send_msg1));
    memcpy(recv->send_msg2,send.send_msg2,strlen(send.send_msg2));
    memcpy(recv->send_msg3,send.send_msg3,strlen(send.send_msg3));
    memcpy(recv->send_msg4,send.send_msg4,strlen(send.send_msg4));
    recv->send_id1 = send.send_id1;
    recv->send_id2 = send.send_id2;
}