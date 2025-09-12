#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main()
{
	const char* host = "www.baidu.com";
	const char* message = 
		"GET / HTTP/1.1\r\n"
        "Host: www.baidu.com\r\n"
        "Connection: close\r\n"
        "User-Agent: MyClient/1.0\r\n"
        "\r\n";
	int sock;
	struct hostent* server;
	struct sockaddr_in serv_addr;
	char buffer[4096];
	int bytes;

	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		printf("socket creation failed !\n");
	}

	// 获取主机地址
	server = gethostbyname(host);
	if( !server )
	{
		printf("host resolution failed\n");
		close(sock);
		return -1;
	}

	// 设置武器地址结构
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(80);
	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

	/* 连接服务器 */
	if(connect(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
	{
		perror("connection failed");
		close(sock);
		exit(1);
	}

	/* 发送GET请求 */
	if(send(sock,message,strlen(message),0) < 0)
	{
		perror("send failed");
		close(sock);
		exit(1);
	}
	printf("get request send \n");
	/* 接收响应 */
	while( (bytes = recv(sock,buffer,sizeof(buffer)-1,0))>0 )
	{
		buffer[bytes] = '\0';
		printf("%s",buffer);
	}

	if(bytes < 0)
	{
		perror("receive error!");
	}

	close(sock);

	return 0;
}
