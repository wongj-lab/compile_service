
#include <string.h>
#include <stdlib.h>
#include "utility.h"

void Free2d(void* p)
{
    void** pp = p;
    
    if( pp && *pp )
    {
        free(*pp);
    }
    
    free(pp);
}

char* FormatByChar(const char* src, char c)  // O(n)
{
    int i = 0;
    int j = 0;
    int len = src ? strlen(src) : 0;
    int flag = 0;
    char* ret = len ? malloc(len + 1) : NULL;
    
    if( ret )
    {
        while( (i < len) && (src[i] == c) ) i++;
        
        while( i < len )
        {
            if( src[i] != c )
            {
                ret[j++] = src[i];
                flag = 0;
            }
            else
            {
                if( !flag )
                {
                    ret[j++] = src[i];
                    flag = 1;
                }
            }
            
            i++;
        }
        
        if( flag ) j--;
        
        ret[j] = 0;
    }
    
    return ret;
}

int DivideByChar(const char* line, char c, char** argv, int row, int col)
{
    int ret = 0;
    
    if( line && argv )
    {
        int i = 0;
        int j = 0;
        char* buf = FormatByChar(line, c);
        int len = buf ? strlen(buf) : 0;
        
        if( len )
        {
            buf[len] = c;
            
            for(i=0, j=0; (i<=len) && (ret<row); i++)
            {
                if( buf[i] == c )
                {
                    int k = (i-j < col) ? i-j : col;
                    
                    strncpy(argv[ret], buf+j, k);
                    
                    argv[ret][(k < col) ? k : (k-1)] = 0;
                    
                    j = i + 1;
                    
                    ret++;
                }
            }
            
            free(buf);
        } 
    }
    
    return ret;
}

int CharCount(const char* s, char c)
{
    int ret = 0;
    
    while( s && *s )
    {
        ret += (*s++ == c);
    }
    
    return ret;
}
