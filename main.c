#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include "tcp_server.h"
#include "utility.h"
#include "page.h"

#define BUF_SIZE 512

typedef struct
{
	const char* cmd;
	void (*handler)(const char*);
}Handler;

enum {STOP,RUN,PAUSE};

static const volatile char* g_root = NULL;
static volatile int g_status = STOP;

static void DoResp(TcpClient* client)
{
	int len = TcpClient_Available(client);

	if( len>0 )
	{
		char* buf = malloc(len+1);

		TcpClient_RecvRaw(client,buf,len);

		buf[len] = 0;

		printf("%s -Request: %s\n",__FUNCTION__,buf);

		char resp[255] = {0};

		sscanf(buf,"GET %s HTTP",resp);

		printf("%s -Req String: %s\n",__FUNCTION__,resp);

		char* format = "HTTP/1.1 200 OK\r\n"
                       "Server:D.T. Http Server\r\n"
                       "Content-Length:%d\r\n"
                       "Content-Type:text/html\r\n"
                       "Connection:close\r\n"
                       "\r\n"
                       "%s";
		sprintf(resp,format,strlen((const char*)g_root),g_root);
		
		TcpClient_SendRaw(client,resp,strlen(resp));

		TcpClient_Del(client);

		free(buf);
	}
}

static void* Process_Thread(void *arg)
{
	while(TcpClient_IsValid(arg))
	{
		DoResp(arg);
	}

	printf("%s - Thread Exit: %p\n",__FUNCTION__,arg);

	return arg;
}

static void* Server_Thread(void* arg)
{
	while( TcpServer_IsValid(arg) )
	{
		TcpClient* client = TcpServer_Accept(arg);

		if( client&&(g_status != PAUSE) )
		{
			pthread_t tid = 0;

			pthread_create(&tid,NULL,Process_Thread,client);
		}
		else
		{
			TcpClient_Del(client);
		}
	}

	g_status = STOP;

	return arg;
}

static void Start_Handler(const char* arg)
{
	int err = 0;

	if(g_status == STOP)
	{
		TcpServer* server = TcpServer_New();

		TcpServer_Start(server,9000,100);

		if(TcpServer_IsValid(server))
		{
			pthread_t tid = 0;

			err = pthread_create(&tid,NULL,Server_Thread,server);
		}
		else
		{
			err = 1;
		}
	}

	if( !err )
	{
		g_status = RUN;

		printf("Server is OK!\n");
	}
	else
	{
		g_status = STOP;

		printf("Server is failed!\n");
	}
}

static void Pause_Handler(const char* arg)
{
	if( g_status == RUN )
	{
		g_status = PAUSE;

		printf("Server is paused!\n");
	}
	else
	{
		printf("Server is NOT started!\n");
	}
}

static void Exit_Handler(const char* arg)
{
	exit(0);
}

static Handler g_handler[] =
{
	{"start",Start_Handler},
	{"pause",Pause_Handler},
	{"exit",Exit_Handler}
};

static void Run(const char* root)
{
	printf("File Server Demo wangjing\n");

	g_root = root;

	while(1)
	{
		char line[BUF_SIZE] = {0};
		int i = 0;

		printf("wangjing @ Input >>>");
		fgets(line,sizeof(line)-1,stdin);

		line[strlen(line)-1] = 0;

		if( *line )
		{
			char* cmd = FormatByChar(line,' ');
			int done = 0;

			for(i=0;i<DIM(g_handler);i++)
			{
				if(strcmp(g_handler[i].cmd,cmd) == 0)
				{
					g_handler[i].handler(cmd);
					done = 1;
					break;
				}
			}

			if( !done )
			{
				printf("\'%s\' can NOT be parsed!\n",cmd);
			}

			free(cmd);
		}
	}
}

int main(int argc,char* argv[])
{
	Table* t = CreateTable();
	RowInfo ri = {"aaa","bbb","ccc","ddd","eee"};
	RowInfo rj = {"111","222","333","444","555"};

	t = InsertRow(t,&ri);
	t = InsertRow(t,&rj);

	char* ts = ToTableString(t);
	char* page = ToPageString("test/path/folder",ts);

	printf("%s\n",page);

	free(page);
	free(ts);
	FreeTable(t);
	return 0;

	if( argc >= 2 )
	{
		DIR* dir = opendir(argv[1]);

		if( dir != NULL )
		{
			closedir(dir);

			Run(argv[1]);
		}
	}

	printf("can not lauch file server,need a valid directory as root. \n");

	return 0;
}
