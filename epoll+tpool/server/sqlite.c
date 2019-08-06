#include "head.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int open_database(char *file_name,sqlite3 **db)
{
	
    int rc = sqlite3_open(file_name,db);
	
    if(rc != SQLITE_OK)
    {
        perror("open sql");
	    sqlite3_close(*db);
	    exit(1);
    }
}

int create_table(sqlite3 *db,char *table_name,int cmd)
{
	int rc = 0;
	char *errmsg = NULL;
	char sql[2 * MAX_SIZE] = {0};
	switch(cmd)
	{
		case 1:
		{
			sprintf(sql,"create table if not exists %s(message text,id1 integer);",table_name);//group chat history,message,sender
			break;
		}

		case 2:
		{
			sprintf(sql,"create table if not exists %s(name text,id1 integer);",table_name);//grouplist name id
			break;
		}

		case 3:
		{
			sprintf(sql,"create table if not exists %s(name text,pswd text,ques text,answ text,id1 integer);",table_name);//userdata
			break;
		}

		case 4:
		{
			sprintf(sql,"create table if not exists %s(message text,id1 integer,id2 integer);",table_name);//message
			break;
		}

		case 5:
		{
			sprintf(sql,"create table if not exists %s(dec text,id1 integer,id2 integer);",table_name);//dec决定是好友还是群
			break;
		}

		case 6:
		{
			sprintf(sql,"create table if not exists %s(dec text,id1 integer,message text);",table_name);//dec决定是成员id还是message
			break;
		}

		case 7:
		{
			//filename sendusername recvusername groupname fileid
			sprintf(sql,"create table if not exists %s(name text,name1 text,name2 text,name3 text,id1 integer);",table_name);
			break;
		}
	}
	
	rc = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
    if(rc != SQLITE_OK)
    {
        perror("create table");
    	sqlite3_free(errmsg);
	    sqlite3_close(db);
	    exit(1);
    }
}

int inport_data(void *para,int n_column,char **column_value,char **column_name)
{
	extern int flag;
	extern Link cb_head;
	
    Link new;

    init_node(&new);
    memset(new,0,sizeof(*new));
   	int c;
   	int temp_id;
   	
   	for(c = 0;c < n_column;c++)
   	{
   		if(strcmp("name",column_name[c]) == 0)
   		{
   			memcpy(new->name,column_value[c],strlen(column_value[c]));
   			
  		}

   		if(strcmp("name1",column_name[c]) == 0)
   		{
   			memcpy(new->name1,column_value[c],strlen(column_value[c]));
   			
  		}

  		if(strcmp("name2",column_name[c]) == 0)
   		{
   			memcpy(new->name2,column_value[c],strlen(column_value[c]));
   			
  		}

  		if(strcmp("name3",column_name[c]) == 0)
   		{
   			memcpy(new->name3,column_value[c],strlen(column_value[c]));
   			
  		}

   		if(strcmp("id1",column_name[c]) == 0)
   		{

   			temp_id = atoi(column_value[c]);
   			
   			new->id1 = temp_id;
   			
   		}

  		if(strcmp("id2",column_name[c]) == 0)
   		{
   			temp_id = atoi(column_value[c]);
   			
   			new->id2 = temp_id;
   			
   		}

   		if(strcmp("pswd",column_name[c]) == 0)
   		{
   			memcpy(new->pswd,column_value[c],strlen(column_value[c]));
   			
   		}

   		if(strcmp("ques",column_name[c]) == 0)
   		{
   			memcpy(new->ques,column_value[c],strlen(column_value[c]));
   			
   		}

   		if(strcmp("answ",column_name[c]) == 0)
   		{
   			memcpy(new->answ,column_value[c],strlen(column_value[c]));
   			
   		}
   		if(strcmp("message",column_name[c]) == 0)
   		{
   			memcpy(new->message,column_value[c],strlen(column_value[c]));
   			
   		}
   		if(strcmp("dec",column_name[c]) == 0)
   		{
   			memcpy(new->dec,column_value[c],strlen(column_value[c]));
   			
   		}
   	}
   	
   	insert_node(new,cb_head);
   	flag++;
    return 0;
}

int count_data(void *para,int n_column,char **column_value,char **column_name)
{
	extern int flag;
	flag++;
	return 0;
}

int exchange_data(void *para,int n_column,char **column_value,char **column_name)
{
	extern int flag;
	extern char temp_name[MAX_SIZE];
	memset(temp_name,0,MAX_SIZE);
	memcpy(temp_name,column_value[0],strlen(column_value[0]));
	
	flag++;
	return 0;
}



int search_data(sqlite3 *db,char *table_name,char *search_column_name,char *search_content,int cmd)
{
	extern int flag;
	extern Link cb_head;
    char *errmsg = NULL;
    char sql[MAX_SIZE] = {0};
    char buf[MAX_SIZE] = {0};
    int temp_id = 0;

    flag = 0;
    cb_head->next = NULL;
    
    if((search_column_name == NULL) || (search_content == NULL))
    {
    	sprintf(sql,"select * from %s;",table_name);
    }

    else 
    {	
    	if((strcmp(search_column_name,"id1") == 0) || (strcmp(search_column_name,"id2") == 0))
    	{
    		temp_id = atoi(search_content);
    		sprintf(sql,"select * from %s where %s=%d;",table_name,search_column_name,temp_id);
    	}

    	else
    	{
    		sprintf(sql,"select * from %s where %s='%s';",table_name,search_column_name,search_content);
    	}
    }

    switch(cmd)
    {
    	case COUNT:
    	{
    		sqlite3_exec(db,sql,count_data,NULL,&errmsg);
    		return flag;
    		break;
    	}

    	case INPORT:
    	{
    		sqlite3_exec(db,sql,inport_data,NULL,&errmsg);
    		return flag;
    		break;
    	}

    	case EXCHANGE:
    	{
    		sqlite3_exec(db,sql,exchange_data,NULL,&errmsg);
    		return flag;
    		break;
    	}
    } 	
}

int search_by_id(sqlite3 *db,char *table_name,char *search_column_name,int id,int cmd)
{
	int ret = 0;
	char buf[MAX_SIZE] = {0};
	sprintf(buf,"%d",id);
	ret = search_data(db,table_name,search_column_name,buf,cmd);
	return ret;
}

int insert_data(sqlite3 *db,char *table_name,struct send_pack recv,int cmd)
{
	char *errmsg = NULL;
	char sql[5 * MAX_SIZE] = {0};
	
	switch(cmd)
	{
		case 1:
		{
			sprintf(sql,"insert into %s(message,id1) values('%s',%d);",table_name,recv.send_msg0,recv.send_id1);
			break;
		}

		case 2:
		{
			sprintf(sql,"insert into %s(name,id1) values('%s',%d);",table_name,recv.sourse_name,recv.send_id1);
			break;
		}

		case 3:
		{
			sprintf(sql,"insert into %s(name,pswd,ques,answ,id1) values('%s','%s','%s','%s',%d);",table_name,recv.sourse_name,recv.send_msg0,recv.send_msg1,recv.send_msg2,recv.send_id1);
			break;
		}

		case 4:
		{
			sprintf(sql,"insert into %s(message,id1,id2) values('%s',%d,%d);",table_name,recv.send_msg0,recv.send_id1,recv.send_id2);
			break;
		}

		case 5:
		{
			sprintf(sql,"insert into %s(dec,id1,id2) values('%s',%d,%d);",table_name,recv.send_msg0,recv.send_id1,recv.send_id2);//friend id group id
			break;
		}

		case 6:
		{
			sprintf(sql,"insert into %s(dec,id1,message) values('%s',%d,'%s');",table_name,recv.send_msg0,recv.send_id1,recv.send_msg1);
			break;
		}
		case 7:
		{
			//filename sendusername recvusername groupname fileid
			sprintf(sql,"insert into %s(name,name1,name2,name3,id1) values('%s','%s','%s','%s',%d);",table_name,recv.sourse_name,recv.send_msg0,recv.send_msg1,recv.send_msg2,recv.send_id1);
		}
	}
	
	
	if(SQLITE_OK != sqlite3_exec(db,sql,NULL,NULL,&errmsg))
    {
	    return FAIL;
    }
    return SUCCESS;	
}


int update_data(sqlite3 *db,char *table_name,char *update_column_name,char *update_content,char *select_column_name,char *select_content,int cmd)
{
	char sql[5 * MAX_SIZE] = {0};
	char *errmsg = NULL;
	int rc = 0;
	int temp_id1 = 0;
	int temp_id2 = 0;

	switch(cmd)
	{
		case 1:
		{
			sprintf(sql,"update %s set %s='%s' where %s='%s';",table_name,update_column_name,update_content,select_column_name,select_content);
			break;
		}

		case 2:
		{
			temp_id1 = atoi(update_content);
			sprintf(sql,"update %s set %s=%d where %s='%s';",table_name,update_column_name,temp_id1,select_column_name,select_content);
			break;
		}

		case 3:
		{
			temp_id1 = atoi(select_content);
			sprintf(sql,"update %s set %s='%s' where %s=%d;",table_name,update_column_name,update_content,select_column_name,temp_id1);
			break;
		}

		case 4:
		{	
			temp_id1 = atoi(update_content);
			temp_id2 = atoi(select_content);
			sprintf(sql,"update %s set %s=%d where %s=%d;",table_name,update_column_name,temp_id1,select_column_name,temp_id2);
			break;
		}
	}
	
	rc = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
	if(rc != SQLITE_OK)
    {
        return FAIL;
    }
    return SUCCESS;
}

int delete_data(sqlite3 *db,char *table_name,char *column_name,char *column_value)
{
	int rc = 0;
	int temp_id = 0;
	char *errmsg = NULL;
	char sql[5 * MAX_SIZE] = {0};

	if((strcmp(column_name,"id1") == 0) || (strcmp(column_name,"id2") == 0))
	{
		temp_id = atoi(column_value);
		sprintf(sql,"delete from %s where %s=%d;",table_name,column_name,temp_id);
	}

	else
	{
		sprintf(sql,"delete from %s where %s='%s';",table_name,column_name,column_value);
	}

	
	rc = sqlite3_exec(db,sql,NULL,NULL,&errmsg);	
	if(rc != SQLITE_OK)
    {
    	perror("update");
        return FAIL;
    }
    return SUCCESS;
}

int delete_by_id(sqlite3 *db,char *table_name,char *column_name,int id)
{
	char buf[MAX_SIZE] = {0};
	int ret;

	sprintf(buf,"%d",id);
	ret = delete_data(db,table_name,column_name,buf);
	return ret;
}

int sqlite3_init(sqlite3 **db)
{
    open_database("./server.db",db);
	create_table(*db,"userdata",3);//所有用户的登陆信息
	create_table(*db,"grouplist",2);//所有的群id name
	create_table(*db,"filelist",7);//文件名 发送者id
	create_table(*db,"message",4);//信息 发送者id 接受者id
	create_table(*db,"friendreq",5);//好友请求发送者 接受者
	create_table(*db,"groupinvite",4);//群名 邀请者 接受者
	//每个用户拥有一个以自己name命名的表，有三列，标识符(群name 好友id)，群id,好友id
	//每个群拥有一个表，用于储存历史消息
}
