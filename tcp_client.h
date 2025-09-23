#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "message.h"

typedef void  TcpClient;

TcpClient* TcpClient_New();
TcpClient* TcpClient_From(int fd);

int TcpClient_SendMsg(TcpClient* client, Message* msg);
int TcpClient_SendRaw(TcpClient* client, char* buf, int length);
Message* TcpClient_RecvMsg(TcpClient* client);
int TcpClient_RecvRaw(TcpClient* client, char* buf, int length);

int TcpClient_Connect(TcpClient* client, char* ip, int port);
int TcpClient_IsValid(TcpClient* client);
void TcpClient_Close(TcpClient* client);
void TcpClient_Del(TcpClient* client);
int TcpClient_Available(TcpClient* client);
void TcpClient_SetData(TcpClient* client, void* data);
void* TcpClient_GetData(TcpClient* client);

int TcpClient_SetOpt(TcpClient* client, int level, int optname, const void* optval, unsigned int optlen);
int TcpClient_GetOpt(TcpClient* client, int level, int optname, void *optval, unsigned int* optlen);

#endif
