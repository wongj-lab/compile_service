#ifndef UTILITY_H
#define UTILITY_H

#include <stdlib.h>

#define Malloc2d(type, row, col)                       \
({                                                     \
    type** ret = NULL;                                 \
                                                       \
    if( (row > 0) && (col > 0) )                       \
    {                                                  \
        type* p = NULL;                                \
                                                       \
        ret = (type**)malloc(row * sizeof(type*));     \
        p = (type*)malloc(row * col * sizeof(type));   \
                                                       \
        if( (ret != NULL) && (p != NULL) )             \
        {                                              \
            int i = 0;                                 \
                                                       \
            for(i=0; i<row; i++)                       \
            {                                          \
                ret[i] = p + i * col;                  \
            }                                          \
        }                                              \
        else                                           \
        {                                              \
            free(ret);                                 \
            free(p);                                   \
                                                       \
            ret = NULL;                                \
        }                                              \
                                                       \
    }                                                  \
                                                       \
    ret;                                               \
})

int CharCount(const char* s, char c);
void Free2d(void* p);
char* FormatByChar(const char* src, char c);
int DivideByChar(const char* line, char c, char** argv, int row, int col);

#endif
