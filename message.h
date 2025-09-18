#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct message
{
    unsigned short type;
    unsigned short cmd;
    unsigned short index;
    unsigned short total;
    unsigned int length;
    unsigned char payload[];
} Message;

Message* Message_New(unsigned short type, unsigned short cmd, unsigned short index, unsigned short total, const char* payload, unsigned int length);
int Message_Size(Message* m);
Message* Message_N2H(Message* m);
Message* Message_H2N(Message* m);
#endif