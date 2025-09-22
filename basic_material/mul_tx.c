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
	struct sockaddr_in  saddr = {0};
	int client = 0;
	struct sockaddr_in remote = {0};
	socklen_t asize = 0;
	int len = 0;
	char buf[32] = "wangjing";
	int r = 0;
	int ttl = 0;
	int loop = 0;
	struct in_addr addr = {0};

	server = socket(PF_INET,SOCK_DGRAM,0);
	if( server == -1 )
	{
		printf("server socket error\n");
		return -1;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(8888);
	
	if( bind(server,(struct sockaddr*)&saddr,sizeof(saddr)) == -1 )
	{
		printf("udp server bind error\n");
		return -1;
	}

	printf("udp server start success\n");

	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr("127.0.0.1");
	remote.sin_port = htons(9000);

	ttl = 0;
	len = sizeof(ttl);
	getsockopt(server,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,&len);
	printf("default ttl = %d\n",ttl);

	ttl = 32;
	len = sizeof(ttl);
	setsockopt(server,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,len);
	printf("current ttl = %d\n",ttl);

	loop = 0;
	len = sizeof(loop);
	getsockopt(server,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,&len);
	printf("defalut loop = %d\n",loop);
	
	addr.s_addr = inet_addr("127.0.0.1");
	len = sizeof(addr);
	setsockopt(server,IPPROTO_IP,IP_MULTICAST_IF,&addr,len);
	printf("current if = %s\n",inet_ntoa(addr));

	while(1)
	{
		len = sizeof(remote);

		r = strlen(buf);

		sendto(server,buf,r,0,(struct sockaddr*)&remote,len);

		sleep(1);
	}
	close(server);

	return 0;
}
