#include "head.h"

int flag;
Link cb_head;
Link online_user;
char temp_name[MAX_SIZE];
sqlite3 *db;
tpool_work_t *temp_work = NULL;
static struct epoll_event ev;
int epfd = 0;

int main(int argc, char **argv)
{

    int pid = 0;
    pid = fork();
    
    if(pid == 0)
    {
        execlp("./udp","udp","NULL");
    }

    if(pid > 0)
    {
        int port = PORT;
        int sockfd = 0;
        int type = 0;
        char buf[1024] = {0};
        //tpool_work_t *temp_work = NULL;
        int connfd = 0;
        int i = 0;
    
        struct sockaddr_in clientaddr;
        struct send_pack temp_recv;

        init_node(&cb_head);
        init_node(&online_user);
        sqlite3_init(&db);

        if (argc>1)
        {
            port = atoi(argv[1]);
        }

        if (tpool_create(THREAD_NUM) != 0) 
        {
            printf("tpool_create failed\n");
            exit(-1);
        }
    

        int listenfd = Server_init(port);
        socklen_t sockaddr_len = sizeof(struct sockaddr);

    
        static struct epoll_event events[EPOLL_SIZE];
        epfd = epoll_create(EPOLL_SIZE);
        ev.events = EPOLLIN;
        ev.data.fd = listenfd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
        struct args *p_args = (struct args *)malloc(sizeof(struct args));
        printf("# Start listening...\n");
        while(1)
        {
            int events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
   
            for(i = 0;i<events_count;i++)
            {
                memset(&ev,0,sizeof(ev));
                if(events[i].data.fd == listenfd)
                {
                    memset(&clientaddr,0,sizeof(clientaddr));
                    connfd = accept(listenfd,(struct sockaddr *)&clientaddr,&sockaddr_len);
                    printf("# New Connection : %d\n",connfd);
                    ev.data.fd=connfd;
                    ev.events=EPOLLIN|EPOLLONESHOT;
                    epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
                }
            
                else if(events[i].events&EPOLLIN)
                {
                    sockfd = events[i].data.fd;
                    memset(&temp_recv,0,sizeof(temp_recv));
                    memset(p_args,0,sizeof(struct args));
                
                    recv_pk(&temp_recv,sockfd);
                    if(temp_recv.send_cmd != 0)
                    {
                        printf("# Revice requestion from %d,cmd is %d\n",sockfd,temp_recv.send_cmd);
                    }
                    if(temp_recv.send_cmd != CLIENT_EXIT)
                    {
                        copy_sendpack(&(p_args->sdpk),temp_recv);
                        init_args(sockfd,temp_recv,p_args);
                
                        temp_work = tpool_add_work(worker,(void*)p_args);
                        #if 0
                        while(1)
                        {
                            if((temp_work->info).flag == NEED_IN)
                            {
                                free(temp_work);
                                ev.events = EPOLLIN|EPOLLONESHOT;
                                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
                                break;
                            }

                            else if((temp_work->info).flag == NEED_OUT)
                            {
                                free(temp_work);
                                ev.events = EPOLLOUT|EPOLLONESHOT;
                                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
                                break; 
                            }
                            else if((temp_work->info).flag == NO_NEED)
                            {
                                free(temp_work);
                                break;
                            }
                        }
                        #endif
                    }
                    else
                    {
                        printf("# Client %d has been out of connection !\n",sockfd);
                        close(sockfd);
                    }
                }
            
                else if(events[i].events&EPOLLOUT)
                {  
                    /*
                    sockfd = events[i].data.fd;
                    type = recv_int(sockfd);
               

                    temp_work = tpool_add_work(worker,(void*)p_args);

                    while(1)
                    {
                        if((temp_work->info).flag == NEED_IN)
                        {
                            free(temp_work);
                        
                            ev.events = EPOLLIN|EPOLLONESHOT;
                            epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
                            break;
                        }

                        else if((temp_work->info).flag == NEED_OUT)
                        {
                            free(temp_work);
                        
                            ev.events = EPOLLOUT|EPOLLONESHOT;
                            epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
                            break; 
                        }
                    }
                    */
                }
            }
        }

    }
    
    while(1)
    {
        sleep(1);
    }
    return 0;
}
