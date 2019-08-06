#ifndef WORK_H__
#define WORK_H__

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
#include <time.h>
#include <termios.h>
#include <assert.h>

#define SERVER_IP   "127.0.0.1"     //server IP
#define PORT 6667                  //Server端口
#define THREAD_NUM  4               //线程池大小
#define FILENAME_MAXLEN   30        //文件名最大长度
#define INT_SIZE    4               //int类型长度

#define SEND_SIZE    8192      	//64K
#define BLOCKSIZE   67108864		//512M
#define RECVBUF_SIZE    8192  
#define CONN_MAX  10  

#define SIGNUP 1011
#define LOGIN 1000
#define CLIENT_EXIT 1001
#define REFINDA 1002
#define REFINDB 1021
#define VIEW 1003
#define DELETE 1004
#define OFFLINE 1005
#define MUTI_SEND 1006
#define SHOW 1007
#define FRIEND_REQ 1008
#define FRIEND_ACC 1009
#define GROUP 1010
#define CHANGE 1012
#define TAN_FILE 1013
#define RECV_FILE 1014
#define EXCHANGE 1015
#define INFO 1016
#define CHECK 1017
#define FRESH 1018
#define FILE_INFO 1019
#define FILE_DATA 1020

#define PRINT 200
#define INPORT 201
#define COUNT 202
#define REMOVE 203
#define MSG 204
#define ID 205

#define SUCCESS 3001
#define FAIL 3002

#define MAX_SIZE 128

/*文件信息*/
struct fileinfo
{
    char filename[FILENAME_MAXLEN];     //文件名
    int filesize;                       //文件大小
    int count;                          //分块数量
    int bs;                             //标准分块大小
};

/*分块头部信息*/
struct head
{
    char filename[FILENAME_MAXLEN];     //文件名
    int id;                             //分块所属文件的id，gconn[CONN_MAX]数组的下标
    int offset;                         //分块在原文件中偏移
    int bs;                             //本文件块实际大小
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

struct send_pack
{ 
    int send_cmd;
    int result;
    char sourse_name[MAX_SIZE];
    char temp_name[MAX_SIZE];
    char send_msg0[MAX_SIZE];
    char send_msg1[MAX_SIZE];
    char send_msg2[MAX_SIZE];
    char send_msg3[MAX_SIZE];
    char send_msg4[MAX_SIZE];
    int send_id1;
    int send_id2;
};

struct current_data
{
    char current_name[MAX_SIZE]; 
    char current_pswd[MAX_SIZE];
    char current_ques[MAX_SIZE];
    char current_answ[MAX_SIZE];
    int current_id1;
    int current_id2;
    int fd;
};

struct node
{
    int fd;//用户登陆时所在客户机的sockfd号
    
    char name[MAX_SIZE];
    char pswd[MAX_SIZE];
    char ques[MAX_SIZE];
    char answ[MAX_SIZE];
    char message[MAX_SIZE];
    char dec[MAX_SIZE];
    int id1;
    int id2;
    
    struct node *next;
};
typedef struct node Node;
typedef struct node *Link;

/*创建大小为size的文件*/
int createfile(char *filename, int size);
/*设置fd非阻塞*/
void set_fd_noblock(int fd);

int Client_init(char *ip);

void send_fileinfo(int sock_fd,char *fname,struct stat* p_fstat,struct fileinfo *p_finfo,int *flag);                

void send_filedata(struct head * p_fhead,int sockfd);

struct head * new_fb_head(char *filename, int freeid, int *offset);

void sendfile(int sockfd);

int send_pk(struct send_pack *pk,int sockfd);
int recv_pk(struct send_pack *pk,int sockfd);
int press_anykey(int a,int b);
void *pthread_recv(void *arg);
int handle_friend(struct send_pack recv_pack);
int signup(int sockfd);
int login(int sockfd);
int client_off(int sockfd);
int refind(int sockfd);
int send_friend_req(int sockfd);
int accept_friend(int sockfd);
void recvfile(int sockfd);
int muti_send(int sockfd);
int view_user(int sockfd);
int mannage_friend(int sockfd);
int mannage_group(int sockfd);
int send_file(int sockfd);
int user_ui();
int check_message(int sockfd);
int change_info(int sockfd);
int fresh_data(int sockfd);
int chat_ui();
int getch();
int passwd_input(char *passwd);
int init_node(Link *new);
int insert_node(Link new,Link head);
int search_node(Link head,Link *object,char *content,int range);
int remove_node(Link head,char *content,int range);
int user_offline(int sockfd);
int recv_fileinfo(int sockfd);
void recv_filedata(int sockfd);
//void udp_send(int sockfd);
void udp_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, int *p_last_bs,struct sockaddr_in *addr,int *len);
void udp_filedata(struct head * p_fhead,int sockfd,struct sockaddr_in *addr,int *len);
void udpfile(int info_fd);
#endif
