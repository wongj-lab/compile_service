#ifndef UDP_POINT_H
#define UDP_POINT_H

#include "message.h"

typedef void  UdpPoint;

UdpPoint* UdpPoint_New();
UdpPoint* UdpPoint_From(int fd);

int UdpPoint_SendMsg(UdpPoint* point, Message* msg, const char* remote, int port);
int UdpPoint_SendRaw(UdpPoint* point, char* buf, int length, const char* remote, int port);
Message* UdpPoint_RecvMsg(UdpPoint* point, char* remote, int* port);
int UdpPoint_RecvRaw(UdpPoint* point, char* buf, int length, char* remote, int* port);

void UdpPoint_Del(UdpPoint* point);
int UdpPoint_Available(UdpPoint* point);
void UdpPoint_SetData(UdpPoint* point, void* data);
void* UdpPoint_GetData(UdpPoint* point);

int UdpPoint_SetOpt(UdpPoint* point, int level, int optname, const void* optval, unsigned int optlen);
int UdpPoint_GetOpt(UdpPoint* point, int level, int optname, void* optval, unsigned int* optlen);

#endif
