#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include "udp_point.h"
#include "msg_parser.h"

typedef struct udp_point
{
	int fd;
	MParser* parser;
	void* data;
}Point;

static char g_temp[1024*4] = {0};

static void ParserAddr(struct sockaddr_in addr,char* ip,int* port)
{
	if( ip )
	{
		strcpy(ip,inet_ntoa(addr.sin_addr));
	}

	if( port )
	{
		*port = ntohs(addr.sin_port);
	}
}

UdpPoint* UdpPoint_New(int port)
{
	Point* ret = malloc(sizeof(Point));
	struct sockaddr_in addr = {0};
	int ok = !!ret;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	ok = ok && ( (ret->parser = MParser_New())!=NULL );
	ok = ok && ( (ret->fd = socket(PF_INET,SOCK_DGRAM,0))!=-1 );
	ok = ok && ( bind(ret->fd,(struct sockaddr*)&addr,sizeof(addr)) != -1 );

	if( ok )
	{
		ret->data = NULL;
	}
	else
	{
		ret?(MParser_Del(ret->parser),NULL):NULL;
		ret?close(ret->fd):-1;

		free(ret);

		ret = NULL;
	}

	return ret;
}
UdpPoint* UdpPoint_From(int fd)
{
	Point* ret = malloc(sizeof(Point));

	if( ret )
	{
		ret->fd = fd;
		ret->parse = MParser_New();
		ret->data = NULL;
	}

	return (ret && ret->parser)?ret:(free(ret),NULL);
}

int UdpPoint_SendMsg(UdpPoint* point,Message* msg,const char* remote,int port)
{
	int ret = 0;
	Point* c = (Point*)point;

	if( c && msg && remote )
	{
		int len = Message_Size(msg);
		char* data = (char*)Message_H2N(msg);

		ret = UdpPoint_SendRaw(c,data,len,remote,port);

		Message_N2H(msg);
	}
}
int UdpPoint_SendRaw(UdpPoint* point,char* buf,int length,const char* remote,int port)
{
	int ret = 0;
	Point* c = (Point*)point;

	if( c && buf && remote )
	{
		struct sockaddr_in raddr = {0};
		int addrlen = sizeof(raddr);

		raddr.sin_family = AF_INET;
		raddr.sin_addr.s_addr = inet_addr(remote);
		raddr.sin_port = htons(port);

		ret = (sendto(c->fd,buf,length,0,(struct sockaddr*)&raddr,addrlen) != -1);
	}

	return ret;
}
Message* UdpPoint_RecvMsg(UdpPoint* point,char* remote,int* port)
{
	Message* ret = NULL;
	Point* c = (Point*)point;

	if( c )
	{
		struct sockaddr_in raddr = {0};
		int addrlen = sizeof(raddr);
		int length = recvfrom(c->fd,g_temp,sizeof(g_temp),MSG_PEEK,(struct sockaddr*)&raddr,&addrlen);
		char* buf = (length>0)?malloc(length):NULL;

		length = recvfrom(c->fd,buf,length,0,(struct sockaddr*)&raddr,&addrlen);

		if( length > 0 )
		{
			ret = MParser_ReadMem(c->parser,buf,length);
		}

		if( ret )
		{
			ParserAddr(raddr,remote,port);
		}

		free(buf);
	}

	return ret;
}
int UdpPoint_RecvRaw(UdpPoint* point,char* buf,int length,char* remote,int* port)
{
	int ret = 0;
	Point* c = (Point*)point;

	if( c && buf )
	{
		struct sockaddr_in raddr = {0};
		int addrlen = sizeof(raddr);
		
		ret = recvfrom(c->fd,buf,length,0,(struct sockaddr*)&raddr,&addrlen);

		if( ret != -1 )
		{
			ParserAddr(raddr,remote,port);
		}
	}

	return ret;
}

void UdpPoint_Del(UdpPoint* point)
{
	Point* c = (Point*)point;

	if( c )
	{
		close( c->fd );
		MParser_Del(c->parser);
		free(c);
	}
}
int UdpPoint_Available(UdpPoint* point)
{
	int ret = -1;
	Point* c = (Point*)point;

	if( c )
	{
		struct sockaddr_in raddr = {0};
		int len = sizeof(raddr);

		ret = recvfrom(c->fd,g_temp,sizeof(g_temp),MSG_PEEK|MSG_DONTWAIT,(struct sockaddr*)&raddr,&len);
	}

	return ret;
}
void UdpPoint_SetData(UdpPoint* point,void* data)
{
	Point* c = (Point*)point;

	if( c )
	{
		c->data = data;
	}
}
void* UdpPoint_GetData(UdpPoint* point)
{
	void* ret = NULL;
	Point* c = (Point*)point;

	if( c )
	{
		ret = c->data;
	}

	return ret;
}

int UdpPoint_SetOpt(UdpPoint* point,int level,int optname,const void* optval,unsigned int optlen)
{
	int ret = -1;
	Point* c = (Point*)point;

	if( c )
	{
		ret = setsockopt(c-fd,level,optname,optval,optlen);
	}

	return ret;
}
int UdpPoint_GetOpt(UdpPoint* point,int level,int optname,void* optval,unsigned int* optlen)
{
	int ret = -1;
	Point* c = (Point*)point;

	if( c )
	{
		ret = getsockopt(c->fd,level,optname,optval,optlen);
	}

	return ret;
}

