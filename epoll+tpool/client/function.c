#include "work.h"

int send_pk(struct send_pack *pk,int sockfd)
{
    
    if(send(sockfd,pk,sizeof(*pk),0) < 0)
    {
		perror("send");
		exit(1);
		return FAIL;
    }
    return SUCCESS;
}

int recv_pk(struct send_pack *pk,int sockfd)
{
    
    if(recv(sockfd,pk,sizeof(*pk),0) < 0)
    {
    	perror("recv");
    	exit(1);
		return FAIL;
    }
    return SUCCESS;
}

int press_anykey(int a,int b)//由用户决定是否执行下一步操作
{
	sleep(0.5);
	if(b == 1)
	{
		
		printf("1 >> Continue ");
		printf("2 >> Exit\n");
		
	}
	if(b == 0)
	{
		printf("> Press any key to continue: \n");
	}
	int buf;
	scanf("%d",&buf);
	fflush(stdin);
	if(a == 1)
	{
		system("clear");
	}
	
	return buf;
}

void *pthread_recv(void *arg)//客户端主进程创建的线程，在用户登录成功以后工作，保持接收服务器的信息
{
    int fd = *((int *)arg);
    struct send_pack recv_pack;
    extern int flag;

    while(1)
    {
    	if(flag == 1)
    	{
        	memset(&recv_pack,0,sizeof(recv_pack));
			recv_pk(&recv_pack,fd);

			switch(recv_pack.send_cmd)
			{
				case FRIEND_REQ://服务器转发过来的好友请求
				{
					handle_friend(recv_pack);
					break;
				}
			
				case MUTI_SEND://其他用户私发过来的信息
				{
					printf("\n%s:%s\n",recv_pack.sourse_name,recv_pack.send_msg0);
					break;
				}

				case INFO:
				{
					printf("%s\n",recv_pack.send_msg0);
					break;
				}
			}
		}
    }
}

int handle_friend(struct send_pack recv_pack)
{
	extern Link friend_req;
	extern Link cb_head;
	Link new;
	init_node(&new);
	new->id1 = recv_pack.send_id1;
	insert_node(new,friend_req);
}

int signup(int sockfd)//向服务器发送一个结构体，包含名字，密码，问题，答案
{
    struct send_pack sign_pack;
    char buf[MAX_SIZE] = {0};
    int len = 0;
    memset(&sign_pack,0,sizeof(sign_pack));
    sign_pack.send_cmd = SIGNUP;
    
    
    
    	memset(&sign_pack,0,sizeof(sign_pack));
    	memset(buf,0,sizeof(buf));
        sign_pack.send_cmd = SIGNUP;
        printf("> Please input your name: ");
        scanf("%s",sign_pack.sourse_name);
        getchar();
        while(1)
        {
        	printf("> Please input your password: ");
 			len = passwd_input(buf);
 			printf("\n");
 			if(len > 15 || len < 6)
 			{
 				printf("# Your password is not suitable ! Please input again !\n");
 			}
 			else
 			{
 				memcpy(sign_pack.send_msg0,buf,len);
 				break;
 			}
 		}
        printf("> Please input your refind-question: ");
        scanf("%s",sign_pack.send_msg1);
        printf("> Please input your refind-answer: ");
        scanf("%s",sign_pack.send_msg2);
        printf("\n");
        send_pk(&sign_pack,sockfd);
        memset(&sign_pack,0,sizeof(sign_pack));
        recv_pk(&sign_pack,sockfd);//接收服务器发送到结构体，获得注册结果
	    if(sign_pack.result == SUCCESS)
	    {
	    	printf("# User %s have successfully sign up , user id : %d\n",sign_pack.sourse_name,sign_pack.send_id1);
            press_anykey(0,0);
			return SUCCESS;
	        
	    }
	    else if(sign_pack.result == FAIL)
	    {
	    	printf("# Your name has already been used !\n");
	    	press_anykey(0,0);
			return FAIL;
	    }
    
}

int login(int sockfd)
{
    extern struct current_data temp_data0;
    struct send_pack log_pack;
    int len = 0;
    char buf[MAX_SIZE] = {0};
    memset(&log_pack,0,sizeof(log_pack));
    memset(&temp_data0,0,sizeof(temp_data0));

    
        log_pack.send_cmd = LOGIN;
		printf("> Please input your name: ");
		scanf("%s",log_pack.sourse_name);
        getchar();
        while(1)
        {
        	printf("> Please input your password: ");
 			len = passwd_input(buf);
 			printf("\n");
 			if(len > 15 || len < 6)
 			{
 				printf("# Your password is not suitable ! Please input again !\n");
 			}
 			else
 			{
 				memcpy(log_pack.send_msg0,buf,len);
 				break;
 			}
 		}
		send_pk(&log_pack,sockfd);
		memset(&log_pack,0,sizeof(log_pack));
		recv_pk(&log_pack,sockfd);
		if(log_pack.result == SUCCESS)
		{
			temp_data0.current_id1 = log_pack.send_id1;
			memcpy(temp_data0.current_name,log_pack.sourse_name,strlen(log_pack.sourse_name));
        	return SUCCESS;
		}
		else if(log_pack.result == FAIL)
		{
	    	printf("Log in fail !\n");
			press_anykey(1,0);
			return FAIL;
		}
    
}

int client_off(int sockfd)//向服务器发送客户端下线消息
{
    struct send_pack exit_pack;
    exit_pack.send_cmd = CLIENT_EXIT;
    send_pk(&exit_pack,sockfd);
    close(sockfd);
}

int refind(int sockfd)
{
    struct send_pack refind_pack;
    char buf[MAX_SIZE] = {0};
	memset(&refind_pack,0,sizeof(refind_pack));
	refind_pack.send_cmd = REFINDA;
	
	
	
	    printf("> Please input your name: ");
		scanf("%s",refind_pack.sourse_name);
		memcpy(buf,refind_pack.sourse_name,strlen(refind_pack.sourse_name));
		send_pk(&refind_pack,sockfd);
		
		memset(&refind_pack,0,sizeof(refind_pack));
		
		recv_pk(&refind_pack,sockfd);
		if(refind_pack.result == SUCCESS)//若姓名匹配，服务器发送问题
		{	
			printf("# Your refind question is %s\n> Please input your answer: ",refind_pack.send_msg0);
			memset(&refind_pack,0,sizeof(refind_pack));
			scanf("%s",refind_pack.send_msg0);
			refind_pack.send_cmd = REFINDB;
			memcpy(refind_pack.sourse_name,buf,strlen(buf));
			send_pk(&refind_pack,sockfd);//发送答案
			
			memset(&refind_pack,0,sizeof(refind_pack));
			
			recv_pk(&refind_pack,sockfd);
			if(refind_pack.result == SUCCESS)//接收结果
			{
				printf("# Refind success,your password is %s\n",refind_pack.send_msg0);
				press_anykey(0,0);
				
			}
		
			else if(refind_pack.result == FAIL)
			{
				printf("# Refind answer is wrong !\n");
				press_anykey(0,0);
			}
		}
		
		else if(refind_pack.result == FAIL)
		{
			printf("# Your user name does not exist !\n");
			press_anykey(0,0);
		}
	
}

int send_friend_req(int sockfd)//发送好友请求
{
	
	struct send_pack frd_pk;
	memset(&frd_pk,0,sizeof(frd_pk));
	frd_pk.send_cmd = FRIEND_REQ;
	printf("> Please input id of which user you want to add friend:\n");
	scanf("%d",&frd_pk.send_id1);
	fflush(stdin);
	send_pk(&frd_pk,sockfd);
}


int accept_friend(int sockfd)//打印储存好友请求信息的链表，向服务器发送接收某id好友请求的消息
{
	
	
	struct send_pack sdpk;
	int temp_id = 0;
	Link p;

	memset(&sdpk,0,sizeof(sdpk));
	sdpk.send_cmd = SHOW;
	sdpk.result = 4;
	send_pk(&sdpk,sockfd);
	

	memset(&sdpk,0,sizeof(sdpk));
	printf("> Please press id to accept requestion: ");
	scanf("%d",&temp_id);
	
	sdpk.send_cmd = FRIEND_ACC;
	sdpk.send_id1 = temp_id;
	send_pk(&sdpk,sockfd);
	
}


int muti_send(int sockfd)
{
	extern struct current_data temp_data0;
	struct send_pack sdpk;
	struct send_pack temp_data;
	int temp_id = 0;
	
	memset(&sdpk,0,sizeof(sdpk));
	memset(&temp_data,0,sizeof(temp_data));
	sdpk.send_cmd = MUTI_SEND;
	memcpy(sdpk.sourse_name,temp_data0.current_name,strlen(temp_data0.current_name));
	sdpk.send_id1 = temp_data0.current_id1;
	printf("> Please input the message you want to send : ");
	scanf("%s",sdpk.send_msg0);
	system("clear");
	printf("1 >> single user\n");
	printf("2 >> user in group\n");
	printf("3 >> every online user\n");
	printf("4 >> every signed user\n");
	printf("# Please input the range you want to send : ");

	scanf("%d",&sdpk.result);
	system("clear");
	switch(sdpk.result)
	{
		case 1:
		{
			view_user(sockfd);
			printf("> Please input id : ");
			scanf("%d",&sdpk.send_id2);
			break;
		}
		case 2:
		{
			temp_data.send_cmd = GROUP;
			send_pk(&temp_data,sockfd);
			memset(&temp_data,0,sizeof(temp_data));
			temp_data.send_cmd = 2;
			send_pk(&temp_data,sockfd);
			memset(&temp_data,0,sizeof(temp_data));

			printf("> Please input the name of group : ");
			scanf("%s",sdpk.sourse_name);
			break;
		}
		case 3:
		{
			break;
		}
		case 4:
		{
			break;
		}
	}
	send_pk(&sdpk,sockfd);	
}

int view_user(int sockfd)
{
	extern struct current_data temp_data0;

	int cmd = 0;

	printf("1 >> View all online user\n");
	printf("2 >> View all signed user\n");
	printf("> Input your choice :\n");

	scanf("%d",&cmd);
	system("clear");
	struct send_pack sdpk;
	memset(&sdpk,0,sizeof(sdpk));
	sdpk.send_id1 = temp_data0.current_id1;
	memcpy(sdpk.sourse_name,temp_data0.current_name,strlen(temp_data0.current_name));
	sdpk.send_cmd = SHOW;
	switch(cmd)
	{
		case 1:
		{
			sdpk.result = 2;
			send_pk(&sdpk,sockfd);
			break;
		}

		case 2:
		{
			sdpk.result = 3;
			send_pk(&sdpk,sockfd);
			break;
		}
	}
	press_anykey(0,0);

}


int mannage_friend(int sockfd)
{
	int cmd = 0;
	int buf = 0;
	system("clear");
	printf("1 >> View all friend\n");
	printf("2 >> View all online friend\n");
	printf("3 >> Add friend\n");
	printf("4 >> Check friend requestion\n");
	printf("5 >> Delete friend\n");
	printf("> Input your choice :\n");
	scanf("%d",&cmd);
	struct send_pack sdpk;
	memset(&sdpk,0,sizeof(sdpk));
	switch(cmd)
	{
		case 1:
		{
			sdpk.send_cmd = SHOW;
			sdpk.result = 1;
			send_pk(&sdpk,sockfd);
			press_anykey(0,0);
			break;
		}
		case 2:
		{
			sdpk.send_cmd = SHOW;
			sdpk.result = 5;
			send_pk(&sdpk,sockfd);
			press_anykey(0,0);
			break;
		}
		case 3:
		{
			while(1)
			{
				view_user(sockfd);
				send_friend_req(sockfd);
				buf = press_anykey(1,1);
				if(buf == 2)
				{
					break;
				} 
			}
			break;
		}
		
		case 4:
		{
			while(1)
			{
				accept_friend(sockfd);
				buf = press_anykey(1,1);
				if(buf == 2)
				{
					break;
				} 
			}			
			break;
		}
		
		case 5:
		{
			sdpk.send_cmd = SHOW;
			sdpk.result = 1;
			send_pk(&sdpk,sockfd);
			memset(&sdpk,0,sizeof(sdpk));
			printf("> Please input the id of the friend you want to delete :\n");
			scanf("%d",&sdpk.send_id1);
			sdpk.send_cmd = DELETE;
			send_pk(&sdpk,sockfd);
			press_anykey(0,0);
			break;
		}
	}
}	

int mannage_group(int sockfd)
{
	system("clear");
	printf("1 >> create a new group\n");
	printf("2 >> view group\n");
	printf("8 >> view your group\n");
	printf("9 >> view people in group\n");
	printf("3 >> join a group\n");
	printf("4 >> invite user to group\n");
	printf("5 >> chatting in a group\n");
	printf("6 >> delete user in a group\n");
	//printf("7 >> keep user silent\n");
	printf("\n> Input your choice:\n");
	
	
	struct send_pack sdpk;
	int cmd;

	memset(&sdpk,0,sizeof(sdpk));
	sdpk.send_cmd = GROUP;
	send_pk(&sdpk,sockfd);
	memset(&sdpk,0,sizeof(sdpk));
	scanf("%d",&sdpk.send_cmd);
	switch(sdpk.send_cmd)
	{
		case 1:
		{
			printf("> Please input the name of group:\n");
			scanf("%s",sdpk.sourse_name);
			fflush(stdin);
			send_pk(&sdpk,sockfd);
			break;
		}
		
		case 2:
		{
			send_pk(&sdpk,sockfd);
			break;
			
		}
		
		case 3:
		{
			printf("> Please input the name of group : ");
			scanf("%s",sdpk.sourse_name);
			send_pk(&sdpk,sockfd);
			break;
		}
		
		case 4:
		{
			printf("> Please input id of which user you want to invite : ");
			scanf("%d",&(sdpk.send_id1));
			printf("> Please input the name of group : ");
			scanf("%s",sdpk.sourse_name);
			send_pk(&sdpk,sockfd);
			break;
		}
		
		case 5:
		{
			break;
		}
		
		case 6:
		{
			break;
		}
		
		case 7:
		{
			break;
		}

		case 8:
		{
			send_pk(&sdpk,sockfd);
			break;
		}

		case 9:
		{
			printf("> Please input the name of group : ");
			scanf("%s",sdpk.sourse_name);
			send_pk(&sdpk,sockfd);
			break;
		}
	}
	
}

int user_ui(int sockfd)
{
       
	int ui_cmd;
    int ret;
	system("clear");
    printf("<<USER MENU>>\n\n");

    printf("1 >> Log in\n");
	printf("2 >> Sign up\n");
	printf("3 >> Refind password\n");
	printf("4 >> Shut down client\n");
	printf("\nYour choice is :\n");
	
    scanf("%d",&ui_cmd);
    system("clear");
	
    switch(ui_cmd)
    {
        case 4:
		{
			while(1)
			{
				ret = press_anykey(1,1);
				if(ret == 2)
				{
					client_off(sockfd);
					exit(1);
					break;
				}
			}
			break;
		}
        case 1:
		{
			while(1)
			{
				ret = login(sockfd);
				if(ret == SUCCESS)
				{
					printf("# Log in successfully,turn to chat menu in 2 sed !\n");
					sleep(2);
					return SUCCESS;
					break;				
				}
			}
			break;
		}
	    case 2:
		{
			while(1)
			{
				ret = signup(sockfd);
				if(ret == SUCCESS)
				{
					break;
				}
			}
			return FAIL;
			break;
		}
	    case 3:
		{
			refind(sockfd);
			return FAIL;
			break;
		}	
    }
}

int check_message(int sockfd)
{
	struct send_pack sdpk;
	memset(&sdpk,0,sizeof(sdpk));
	sdpk.send_cmd = CHECK;
	printf("1 >> Unchecked message from group\n");
	printf("2 >> Unchecked message from single user\n");
	printf("3 >> Checked message from group\n");
	printf("4 >> Checked message from single user\n");
	printf("> Please input your choice :\n");
	scanf("%d",&sdpk.send_id1);
	system("clear");
	send_pk(&sdpk,sockfd);
}

int change_info(int sockfd)
{
	system("clear");
	printf("1 >> Change name\n");
	printf("2 >> Change password\n");
	printf("3 >> Change refind question\n");
	printf("4 >> Change refind answer\n");

	struct send_pack sdpk;
	int len = 0;
	char buf[MAX_SIZE] = {0};

	memset(&sdpk,0,sizeof(sdpk));
	sdpk.send_cmd = CHANGE;
	printf("> Please input your choice :\n");
	scanf("%d",&sdpk.send_id1);
	if(sdpk.send_id1 == 2)
	{
		getchar();
        while(1)
        {
        	printf("> Please input your new password: ");
 			len = passwd_input(buf);
 			printf("\n");
 			if(len > 15 || len < 6)
 			{
 				printf("# Your password is not suitable ! Please input again !\n");
 			}
 			else
 			{
 				memcpy(sdpk.send_msg0,buf,strlen(buf));
 				break;
 			}
 		}
	}
	else
	{
		printf("> Please input the content you want to update : ");
		scanf("%s",sdpk.send_msg0);
	}
	send_pk(&sdpk,sockfd);
}

int fresh_data(int sockfd)
{	
	extern int flag;
	extern struct current_data temp_data0;
	struct send_pack sdpk;
	struct current_data temp_data;

	memset(&sdpk,0,sizeof(sdpk));
	memset(&temp_data,0,sizeof(temp_data));
	memset(&temp_data0,0,sizeof(temp_data0));

	sdpk.send_cmd = FRESH;
	send_pk(&sdpk,sockfd);
	flag = 0;
	recv(sockfd,&temp_data,sizeof(temp_data),0);
	memcpy(temp_data0.current_name,temp_data.current_name,strlen(temp_data.current_name));
	temp_data0.current_id1 = temp_data.current_id1;
	flag = 1;
	printf("Your current data has been freshed !\n");
	sleep(1);

}

int user_offline(int sockfd)
{
	
	extern struct current_data temp_data0;
	struct send_pack sdpk;
	memset(&sdpk,0,sizeof(sdpk));
	sdpk.send_cmd = OFFLINE;
	memcpy(sdpk.sourse_name,temp_data0.current_name,strlen(temp_data0.current_name));
	sdpk.send_id1 = temp_data0.current_id1;
	send_pk(&sdpk,sockfd);
}

int chat_ui(int sockfd)
{
	extern struct current_data temp_data0;
	extern int flag;
    int buf;
	int ui_cmd = 0;
    int cmd = 0;
    int ret = 0;
    
    system("clear");
    printf("<< CHAT MENU >>\n\n");
	printf("> current user : %s\n",temp_data0.current_name);
	printf("> current id : %d\n\n",temp_data0.current_id1);
    
    printf("1 >> User goes offline\n");
	printf("2 >> Change user information\n");
	printf("3 >> View user\n");
	printf("4 >> Mannage friend\n");
    printf("5 >> Mannage message\n");
	printf("6 >> Mannage group\n");
	printf("7 >> Mannage file\n");
	printf("8 >> Fresh\n");
	
	printf("\n");
	printf("> Your choice is :\n");
	
	flag = 1;
    scanf("%d",&ui_cmd);
	system("clear");
    switch(ui_cmd)
    {
        case 1:
		{
			
			printf("Do you really want to go offline?\n1 >> Yes\n2 >> No\nYour choice is : ");
			scanf("%d",&ui_cmd);
			if(ui_cmd == 1)
			{
				return SUCCESS;
				break;
			}
			else
			{
				return FAIL;
				break;
			}
		}				 
		
		case 5:
		{
			printf("1 >> Send one message\n");
			printf("2 >> Check message\n");
			printf("> Please input your choice :\n");
			scanf("%d",&cmd);
			switch(cmd)
			{
				case 1:
				{
					muti_send(sockfd);
					break;
				}

				case 2:
				{
					check_message(sockfd);
					break;
				}
			}
			press_anykey(0,0);
			break;
		}
		case 2:
		{
	        change_info(sockfd);
	        press_anykey(1,0);
			break;
		}
		case 4:
		{
			mannage_friend(sockfd);
			press_anykey(0,0);
			break;
		}
		case 6:
		{
			mannage_group(sockfd);
			press_anykey(0,0);
			break;
		}
		case 7:
		{
			printf("1 >> Send file\n");
			printf("2 >> Recive file\n");
			printf("> Please input your choice :\n");
			scanf("%d",&cmd);
			switch(cmd)
			{
				case 1:
				{	
					while(1)
					{
						sendfile(sockfd);
						ret = press_anykey(1,1);
						if(ret == 2)
						{
							break;
						}
					}
					break;
				}

				case 2:
				{
					while(1)
					{
						recvfile(sockfd);
						ret = press_anykey(1,1);
						if(ret == 2)
						{
							break;
						}
					}
					break;
				}
			}
			//press_anykey(0,0);
			break;
		}

		case 3:
		{
			while(1)
			{
				view_user(sockfd);
				ret = press_anykey(1,1);
				if(ret == 2)
				{
					break;
				}
			}
			break;
		}

		case 8:
		{
			//fresh_data(sockfd);
			//udpfile(sockfd);
			break;
		}
    }
}

int getch()
{
	int c=0;
	struct termios org_opts, new_opts;
	int res=0;

	res=tcgetattr(STDIN_FILENO, &org_opts);
	assert(res==0);
	memcpy(&new_opts, &org_opts, sizeof(new_opts));
	new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
	c=getchar();
	res=tcsetattr(STDIN_FILENO, TCSANOW, &org_opts);
	assert(res==0);
	return c;
}


int passwd_input(char *passwd)//返回密码长度
{
	int i = 0;
	int len = 0;
	for(i=0;;i++)
	{
		passwd[i]=getch();
		if(passwd[i]=='\n')
		{
			passwd[i]='\0';
			break;
		}
		if(passwd[i]==19)
		{
			printf("\b \b");
			i=i-2;
		}
		else
			printf("*");
		if(i<0)
			passwd[0]='\0';
	}
	len = strlen(passwd);
	return len;

}