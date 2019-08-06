#include "work.h"

int port=PORT;  //默认Port
char *mbegin;   //map起始地址
Link friend_req = NULL;
int flag = 0;
struct current_data temp_data0;
struct current_data temp_data1;

int main(int argc, char **argv)
{
    
    if(argc>1)
    {
        port = atoi(argv[1]);
    }

    int info_fd = Client_init(SERVER_IP);
    init_node(&friend_req);
    pthread_t id;
    pthread_create(&id,NULL,pthread_recv,(void *)(&info_fd));
    int ret0,ret1;
    
    while(1)
    {
        ret0 = user_ui(info_fd);
        if(ret0 == SUCCESS)
        {
            while(1)
            {
                ret1 = chat_ui(info_fd);
                if(ret1 == SUCCESS)
                {
                    user_offline(info_fd);
                    break;
                }
            }
        }
    }
    return 0;
}