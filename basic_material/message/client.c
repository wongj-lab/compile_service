#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "message.h"

int main()
{
	int sock = 0;
	struct sockaddr_in addr = {0};
	int len = 0;
	char buf[128] = {0};
	char input[32] = {0};
	int r = 0;
	Message* pm = NULL;

	sock = socket(PF_INET,SOCK_STREAM,0);

	if(sock == -1)
	{
		printf("socket error\n");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(8888);

	if(connect(sock,(struct sockaddr*)&addr,sizeof(addr)) == -1)
	{
		printf("connect error\n");
		return -1;
	}
	printf("connect success!\n");

	pm = Message_New(0,0,1,3,"A",1);
	send(sock,pm,sizeof(Message)+1,0);

	close(sock);

	return 0;
}
