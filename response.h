#ifndef RESPONSE_H
#define RESPONSE_H

#include "tcp_client.h"

int RequestHandler(TcpClient* client,const char* req,const char* root);

#endif

