#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sqlite3.h>

#define PORT 6667                 //监听端口
#define LISTEN_QUEUE_LEN 100        //listen队列长度
#define THREAD_NUM  8               //线程池大小
#define CONN_MAX  10                //支持最大连接数，一个连接包含多个socket连接（多线程）
#define EPOLL_SIZE  50              //epoll最大监听fd数量
#define FILENAME_MAXLEN   30        //文件名最大长度
#define INT_SIZE    4               //int类型长度

#define UNFINISHED 0            //任务链表的flag参数 0代表未完成
#define NEED_IN 1               //1代表完成了，需要服务器去监听此cfd的发送行为
#define NEED_OUT 2              //2代表完成了，需要服务器去监听此cfd的接收行为
#define NO_NEED 3

#define RECVBUF_SIZE    65536       //64K
#define SEND_SIZE    8192         //64K
#define BLOCKSIZE   67108864        //512M

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
    char name1[MAX_SIZE];
    char name2[MAX_SIZE];
    char name3[MAX_SIZE];
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

//与客户端关联的连接，每次传输建立一个，在多线程之间共享
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

/*线程参数*/
struct args
{
    int fd;
    int flag;
    struct send_pack sdpk;
    void (*handle1)(int fd,struct send_pack recv_pack);
    void (*handle2)(int fd);
    void (*handle3)(int fd);
};

typedef struct tpool_work 
{
    void *  (*routine)(void*);       /* 任务函数 */
    //void                *arg;  
    struct args info;                /* 传入任务函数的参数 */
    struct tpool_work   *next;
}tpool_work_t;

/* 线程池 */
typedef struct tpool 
{
    int             shutdown;                    /* 线程池是否销毁 */
    int             max_thr_num;                 /* 最大线程数 */
    pthread_t       *thr_id;                     /* 线程ID数组首地址 */
    tpool_work_t    *queue_head;                 /* 任务链表队首 */
	tpool_work_t    *queue_tail; 				 /* 任务链表队尾 */
    pthread_mutex_t queue_lock;
    pthread_cond_t  queue_ready;
}tpool_t;

int createfile(char *filename, int size);

int Server_init(int port);

void set_fd_noblock(int fd);

void recv_fileinfo(int sockfd,struct send_pack recv_pack);

void recv_filedata(int sockfd,struct send_pack recv_pack);

void * worker(void *argc);

int recv_int(int sockfd);

void init_args(int sockfd,struct send_pack recv,struct args *p_args);

int tpool_create(int max_thr_num);

void tpool_destroy();

tpool_work_t *tpool_add_work(void*(*routine)(void*), void *arg);

int open_database(char *file_name,sqlite3 **db);

int create_table(sqlite3 *db,char *table_name,int cmd);

int inport_data(void *para,int n_column,char **column_value,char **column_name);

int count_data(void *para,int n_column,char **column_value,char **column_name);

int exchange_data(void *para,int n_column,char **column_value,char **column_name);

int search_data(sqlite3 *db,char *table_name,char *search_column_name,char *search_content,int cmd);

int search_by_id(sqlite3 *db,char *table_name,char *search_column_name,int id,int cmd);

int insert_data(sqlite3 *db,char *table_name,struct send_pack recv,int cmd);

int update_data(sqlite3 *db,char *table_name,char *update_column_name,char *update_content,char *select_column_name,char *select_content,int cmd);

int delete_data(sqlite3 *db,char *table_name,char *column_name,char *column_value);

int delete_by_id(sqlite3 *db,char *table_name,char *column_name,int id);

int sqlite3_init();

int init_node(Link *new);

int insert_node(Link new,Link head);

int search_node(Link head,Link *object,char *content,int range);

int remove_node(Link head,char *content,int range);

int send_pk(struct send_pack *pk,int sockfd);

int recv_pk(struct send_pack *pk,int sockfd);

int handle_show(int cfd,struct send_pack recv);

int press_anykey(int a,int b);

int handle_change(struct send_pack recv,int cfd,struct current_data *temp);

void handle_signup(int cfd,struct send_pack recv_pack);

int handle_login(int cfd,struct send_pack recv_pack);

int handle_refindA(int cfd,struct send_pack recv_pack);

int handle_refindB(int cfd,struct send_pack recv_pack);

int handle_send(int cfd,struct send_pack recv_pack);

int handle_accept_friend(struct send_pack recv_pk,int cfd,struct current_data temp);

int handle_add_friend(struct send_pack recv_pk,int cfd,struct current_data temp);

int handle_group(int cfd,struct current_data temp);

int print_online_user();

int handle_message(int cfd,struct send_pack recv,struct current_data temp);

int handle_file(int cfd,char *file_name,struct current_data temp);

int handle_fresh(int cfd,struct current_data temp);

void copy_sendpack(struct send_pack *recv,struct send_pack send);

int handle_offline(int cfd,struct send_pack recv);

int handle_recv(int cfd,struct send_pack recv);

void send_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, int *p_last_bs);

void send_filedata(struct head * p_fhead,int sockfd);

struct head * new_fb_head(char *filename, int freeid, int *offset);
