
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "msg_parser.h"

typedef struct msg_parser
{
    Message cache;   // 缓存已解析的消息头
    int header;      // 标识消息头是否解析成功
    int need;        // 标识还需要多少字节才能完成解析
    Message* msg;    // 解析中的协议消息（半成品）
} MsgParser;

static void InitState(MsgParser* p)
{
    p->header = 0;
    p->need = sizeof(p->cache);
        
    free(p->msg);

    p->msg = NULL;
}

static int ToMidState(MsgParser* p)
{
    p->header = 1;
    p->need = p->cache.length;

    p->msg = malloc(sizeof(p->cache) + p->need);

    if( p->msg )
    {
        *p->msg = p->cache; 
    }

    return !!p->msg;
}

static Message* ToLastState(MsgParser* p)
{
    Message* ret = NULL;

    if( p->header && !p->need )
    {
        ret = p->msg;

        p->msg = NULL;
    } 

    return ret;
}

static int ToRecv(int fd, char* buf, int size)
{
    int retry = 0;
    int i = 0;

    while( i < size )
    {
        int len = read(fd, buf + i, size - i);
        
        if( len > 0 )
        {
            i += len;
        }
        else if( len < 0 )
        {
            break;
        }
        else
        {
            if( retry++ > 5 )
            {
                break;
            }

            usleep(200 * 1000);
        }
    }

    return i;
}

MParser* MParser_New()
{
    MParser* ret = calloc(1, sizeof(MsgParser));

    if( ret )
    {
        InitState(ret);
    }
    
    return ret;
}

Message* MParser_ReadMem(MParser* parser, unsigned char* mem, unsigned int length)
{
    Message* ret = NULL;
    MsgParser* p = (MsgParser*)parser;

    if( p && mem && length )
    {
        if( !p->header )
        {
            int len = (p->need < length) ? p->need : length;
            int offset = sizeof(p->cache) - p->need;
            
            memcpy((char*)&p->cache + offset, mem, len);

            if( p->need == len )
            {
                Message_N2H(&p->cache);

                mem += p->need;
                length -= p->need;

                if( ToMidState(p) )
                {
                    ret = MParser_ReadMem(p, mem, length);
                }
                else
                {
                    InitState(p);
                }
            }
            else
            {
                p->need -= len;
            }
        }
        else
        {
            if( p->msg )
            {
                int len = (p->need < length) ? p->need : length;
                int offset = p->msg->length - p->need;

                memcpy(p->msg->payload + offset, mem, len);

                p->need -= len;
            }

            if( ret = ToLastState(p) )
            {
                InitState(p);
            }
        }
    }

    return ret;
}

Message* MParser_ReadFd(MParser* parser, int fd)
{
    Message* ret = NULL;
    MsgParser* p = (MsgParser*)parser;

    if( (fd != -1) && p )
    {
        if( !p->header )
        {
            int offset = sizeof(p->cache) - p->need;
            int len = ToRecv(fd, (char*)&p->cache + offset, p->need);
            
            if( len == p->need )
            {
                Message_N2H(&p->cache);

                if( ToMidState(p) )
                {
                    ret = MParser_ReadFd(p, fd);
                }
                else
                {
                    InitState(p);
                }
            }
            else
            {
                p->need -= len;
            }
        }
        else
        {
            if( p->msg )
            {
                int offset = p->msg->length - p->need;
                int len = ToRecv(fd, p->msg->payload + offset, p->need);

                p->need -= len;
            }

            if( ret = ToLastState(p) )
            {
                InitState(p);
            }
        }
    }
    
    return ret;
}

void MParser_Reset(MParser* parser)
{
    MsgParser* p = (MsgParser*)parser;

    if( p )
    {
        InitState(p);
    }
}

void MParser_Del(MParser* parser)
{
    MsgParser* p = (MsgParser*)parser;

    if( p )
    {
        free(p->msg);
        free(p);
    }
}

