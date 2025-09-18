#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "msg_parser.h"

typedef struct msg_parser
{
	Message cache;
	int header;
	int need;
	Message* msg;
}MsgParser;

static void InitState(MsgParser* p)
{
	p->header = 0;
	p->need = sizeof(p->cache);

	free(p->msg);

	p->msg = NULL;
}

static int ToMidState(MsgParser* p)
{
	p->header = 1;
	p->need = p->cache.length;

	p->msg = malloc(sizeof(p->cache) + p->need);

	if( p->msg )
	{
		*p->msg = p->cache;
	}

	return !!p->msg;
}

static Message* ToLastState(MsgParser* p)
{
	Message* ret = NULL;

	if( p->header && !p->need )
	{
		ret = p->msg;
		p->msg = NULL;
	}

	return ret;
}

static void ntoh(Message* m)
{
	m->type = ntohs(m->type);
	m->cmd= ntohs(m->cmd);
	m->index= ntohs(m->index);
	m->total= ntohs(m->total);
	m->length= ntohl(m->length);
}

static int ToRecv(int fd,char* buf,int size)
{
	int retry = 0;
	int i = 0;

	while( i<size )
	{
		int len = read(fd,buf+i,size-i);
		if( len > 0 )
		{
			i += len;
		}
		else if(len < 0)
		{
			break;
		}
		else
		{
			if( retry++ >5 )
			{
				break;
			}

			usleep(200*1000);
		}
	}


	return i;
}


MParser* MParser_New()
{
	MParser* ret = calloc(1,sizeof(MsgParser));

	MParser_Reset(ret);

	return ret;
}


Message* MParser_ReadMem(MParser* parser,unsigned char* mem,unsigned int length)
{
	Message* ret = NULL;
	MsgParser* p = (MsgParser*)parser;

	if( p&&mem&&length )
	{
		if(!p->header)
		{
			if(p->need <= length)
			{
				memcpy(&p->cache,mem,p->need);

				p->cache.type = ntohs(p->cache.type);
				p->cache.cmd = ntohs(p->cache.cmd);
				p->cache.index = ntohs(p->cache.index);
				p->cache.total = ntohs(p->cache.total);
				p->cache.length = ntohl(p->cache.length);

				mem += p->need;
				length -= p->need;

				p->header = 1;
				p->need = p->cache.length;

				ret = MParser_ReadMem(p,mem,length);
			}
		}
		else
		{
			if(!p->msg)
			{
				p->msg = malloc(sizeof(p->cache)+p->need);

				if( p->msg )
				{
					*p->msg = p->cache;
				}
			}

			if( p->msg )
			{
				unsigned int len = (p->need < length)?p->need:length;
				unsigned int offset = p->msg->length - p->need;
				memcpy(p->msg->payload,mem,len);

				p->need -= len;
			}

			if(!p->need)
			{
				ret = p->msg;

				p->msg = NULL;

				MParser_Reset(p);
			}
		}
	}

	return ret;
}

Message* MParser_ReadFd(MParser* parser,int fd)
{
	Message* ret = NULL;
	MsgParser* p = (MsgParser*)parser;

	if( (fd!=-1)&&(p) )
	{
		if(!p->header)
		{
			int offset = sizeof(p->cache) - p->need;
			int len = ToRecv(fd,(char*)&p->cache+offset,p->need);

			if(len == p->need) //包头接收完毕
			{
				ntoh(&p->cache);

				if( ToMidState(p) )
				{
					ret = MParser_ReadFd(p,fd);
				}
				else
				{
					InitState(p);
				}
			}
			else
			{
				p->need -= len;
			}
		}
		else
		{
			if( p->msg )
			{
				int offset = p->msg->length - p->need;
				int len = ToRecv(fd,p->msg->payload+offset,p->need);

				p->need -= len;
			}

			if( ret = ToLastState(p) )
			{
				InitState(p);
			}
		}
	}

	return ret;
}

void MParser_Reset(MParser* parser)
{
	MsgParser* p = (MsgParser*)parser;

	if( p )
	{
		p->header = 0;
		p->need = sizeof(p->cache);

		if(p->msg)
		{
			free(p->msg);
		}

		p->msg = NULL;
	}
}

void MParser_Del(MParser* parser)
{
	MsgParser* p = (MsgParser*)parser;

	if( p )
	{
		if( p->msg )
		{
			free(p->msg);
			free(p);
		}
	}
}


