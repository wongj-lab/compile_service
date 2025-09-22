#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
	int sock = 0;
	struct sockaddr_in addr={0};
	struct sockaddr_in remote = {0};
	struct ip_mreq group = {0};
	int len = 0;
	char buf[128] = {0};
	char input[32] = {0};
	int r = 0;

	sock = socket(PF_INET,SOCK_DGRAM,0);
	if(sock == -1)
	{
		printf("socket error\n");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	if( bind(sock,(struct sockaddr*)&addr,sizeof(addr)) == -1 )
	{
		printf("udp bind error\n");
		return -1;
	}

	group.imr_multiaddr.s_addr = inet_addr("127.0.0.1");
	group.imr_interface.s_addr = htonl(INADDR_ANY);

	setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&group,sizeof(group));

	while(1)
	{
		len = sizeof(remote);

		r = recvfrom(sock,buf,sizeof(buf),0,(struct sockaddr*)&remote,&len);

		if( r>0 )
		{
			buf[r] = 0;
			printf("Receive: %s\n",buf);
		}
		else
		{
			break;
		}
	}

	close(sock);

	return 0;
}
