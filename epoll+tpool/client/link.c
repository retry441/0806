#include "work.h"

int init_node(Link *new)
{
	memset(new,0,sizeof(Link));
    (*new) = (Link)malloc(sizeof(Node));
    (*new)->next = NULL;
        
}

int insert_node(Link new,Link head)
{
    new->next = head->next;
    head->next = new;
}

int search_node(Link head,Link *object,char *content,int range)
{
	Link p;
	p = head->next;
	int cmp = 0;

	switch(range)
	{
		case 1://id
		{
			cmp = atoi(content);
			while(p != NULL)
			{
				if(cmp == p->id1)
				{
					*object = p;
				}
				p = p->next;
			}
			break;
		}

		case 2://fd
		{
			cmp = atoi(content);
			while(p != NULL)
			{
				if(cmp == p->fd)
				{
					*object = p;
				}
				p = p->next;
			}
			break;
		}

		case 3://name
		{
			while(p != NULL)
			{
				if(memcmp(content,p->name,MAX_SIZE))
				{
					*object = p;
				}
				p = p->next;
			}
			break;
		}
	}
}

int remove_node(Link head,char *content,int range)
{
	Link p = NULL;
	
	search_node(head,&p,content,range);
	Link q = NULL;
	q = head->next;

	while(1)
	{
		if(q->next == p)
		{
			break;
		}
		q = q->next;
	}
	q->next = p->next;
}