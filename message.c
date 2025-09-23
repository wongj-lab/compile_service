#include "message.h"
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>

Message* Message_New(unsigned short type, unsigned short cmd, unsigned short index, unsigned short total, const char* payload, unsigned int length)
{
    Message* ret = malloc(sizeof(Message) + length);

    if( ret  )
    {
        ret->type = type;
        ret->cmd = cmd;
        ret->index = index;
        ret->total = total;
        ret->length = length;

        if( payload )
        {
            memcpy(ret + 1, payload, length);
        }
    }

    return ret;
}

int Message_Size(Message* m)
{
    int ret = 0;

    if( m )
    {
        ret = sizeof(Message) + m->length;
    }

    return ret;
}

Message* Message_N2H(Message* m)
{
    if( m )
    { 
        m->type = ntohs(m->type);
        m->cmd = ntohs(m->cmd);
        m->index = ntohs(m->index);
        m->total = ntohs(m->total);
        m->length = ntohl(m->length);
    }

    return m;
}

Message* Message_H2N(Message* m)
{
    if( m )
    {
        m->type = htons(m->type);
        m->cmd = htons(m->cmd);
        m->index = htons(m->index);
        m->total = htons(m->total);
        m->length = htonl(m->length);
    }

    return m;
}
