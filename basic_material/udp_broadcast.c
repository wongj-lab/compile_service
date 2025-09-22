#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>



int main()
{
	int server = 0;
	struct sockaddr_in saddr = {0};
	int client = 0;
	struct sockaddr_in remote = {0};
	socklen_t aszie = 0;
	int len = 0;
	char buf[32] = "wangjing";
	int r = 0;
	int brd = 1;

	server = socket(PF_INET,SOCK_DGRAM,0);
	if( server == -1 )
	{
		printf("server socket error\n");
		return -1;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(8888);
	
	if( bind(server,(struct sockaddr*)&saddr,sizeof(saddr)) ==-1 )
	{
		printf("udp server bind error\n");
		return -1;
	}

	printf("udp server start success\n");

	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = 0xffffffff;
	remote.sin_port = htons(9000);

	setsockopt(server,SOL_SOCKET,SO_BROADCAST,&brd,sizeof(brd));

	while(1)
	{
		len = sizeof(remote);

		r = strlen(buf);

		sendto(server,buf,r,0,(struct sockaddr*)&remote,len);
		sleep(1);
	}
	return 0;
}
