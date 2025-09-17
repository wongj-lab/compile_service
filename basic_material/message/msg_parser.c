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


