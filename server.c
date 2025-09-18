#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include "tcp_client.h"
#include "msg_parser.h"

int main()
{
    int server = 0;
    struct sockaddr_in saddr = {0};
    int client = 0;
    struct sockaddr_in caddr = {0};
    socklen_t asize = 0;
    int len = 0;
    char buf[32] = {0};
    int r = 0;
    MParser* parser = MParser_New();

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
        printf("server bind error\n");
        return -1;
    }

    printf("server start success\n");

    while( 1 )
    {
        TcpClient* c = NULL;

        asize = sizeof(caddr);

        client = accept(server, (struct sockaddr*)&caddr, &asize);

        if( client == -1 )
        {
            printf("client accept error\n");
            return -1;
        }

        c = TcpClient_From(client);

        printf("client: %d\n", client);
        printf("addr: %p\n", c);

        do
        {
            Message* m = TcpClient_RecvMsg(c);

            if( m )
            {
                // printf("type = %d\n", m->type);
                // printf("cmd = %d\n", m->cmd);
                // printf("index = %d\n", m->index);
                // printf("total = %d\n", m->total);
                // printf("length = %d\n", m->length);
                printf("payload = %s\n", m->payload);
                
                free(m);
            }

        } while ( TcpClient_IsValid(c) );

        printf("client socket is closed\n");

        TcpClient_Del(c);
    }
    
    close(server);

    return 0;
}
