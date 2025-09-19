#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int server_handler(int server)
{
	struct sockaddr_in addr = {0};
	socklen_t asize = sizeof(addr);

	return accept(server,(struct sockaddr*)&addr,&asize);
}

int client_handler(int client)
{
	char buf[32] = {0};
	int ret = recv(client,buf,sizeof(buf)-1,0);

	if( ret > 0 )
	{
		buf[ret] = 0;
		printf("Receive: %s \n",buf);
	}
	return ret;
}

int main()
{
	int server = 0;
    struct sockaddr_in saddr = {0};
    int max = 0;
    int num = 0;
    fd_set reads = {0};
    fd_set temps = {0};
    fd_set except = {0};
    struct timeval timeout = {0};

    server = socket(PF_INET, SOCK_STREAM, 0);

    if( server == -1 )
    {
        printf("server socket error\n");
        return -1;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(8888);

    if( bind(server, (struct sockaddr*)&saddr, sizeof(saddr)) == -1 )
    {
        printf("server bind error\n");
        return -1;
    }

    if( listen(server, 1) == -1 )
    {
        printf("server listen error\n");
        return -1;
    }

    printf("server start success\n");

    FD_ZERO(&reads);
    FD_SET(server, &reads);

    max = server;

	while(1)
	{
		temps = reads;
		except = reads;
		
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		num = select(max+1,&temps,0,&except,&timeout);

		if( num > 0 )
		{
			int i = 0;

			for(i=0;i<=max;i++)
			{
				if(FD_ISSET(i,&except))
				{
					if( i != server )
					{
						char buf[2] = {0};
						int r = recv(i,buf,sizeof(buf),MSG_OOB);

						if( r > 0 )
						{
							buf[r] = 0;
							printf("OOB: %s\n",buf);
						}
					}
				}

				if(FD_ISSET(i,&temps))
				{
					if( i==server )
					{
						int client = server_handler(server);

						if(client > -1)
						{
							FD_SET(client,&reads);

							max = (client > max)?client:max;

							printf("accept client :%d\n",client);
						}
					}
					else
					{
						int r = client_handler(i);

						if(r == -1)
						{
							FD_CLR(i,&reads);
							close(i);
						}
					}
				}
			}
		}

	}
	return 0;
}
