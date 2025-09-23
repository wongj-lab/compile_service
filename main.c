#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include "tcp_server.h"
#include "utility.h"

void func1()
{
    TcpServer* s = TcpServer_New();

    TcpServer_Start(s, 9000, 100);

    if( TcpServer_IsValid(s) )
    {
        char buf[64] = {0};
        int len = 0;
        TcpClient* c = TcpServer_Accept(s);

        if( c )
        {
            len = TcpClient_RecvRaw(c, buf, sizeof(buf)-1);

            buf[len] = 0;

            printf("recv = %s\n", buf);

            TcpClient_SendRaw(c, buf, len);

            TcpClient_Del(c);
        }

        TcpServer_Del(s);
    }
}

void* thread_entry(void* arg)
{
    pthread_t id = pthread_self();
    int n = (long)arg;
    int i = 0;

    for(i=0; i<n; i++)
    {
        printf("tid = %ld, i = %d\n", id, i);
        sleep(1);
    }

    return NULL;
}

void func2()
{
    pthread_t t1 = 0;
    pthread_t t2 = 0;
    int arg1 = 5;
    int arg2 = 10;

    printf("create thread...\n");

    pthread_create(&t1, NULL, thread_entry, (void*)arg1);
    pthread_create(&t2, NULL, thread_entry, (void*)arg2);

    printf("t1 = %ld\n", t1);
    printf("t2 = %ld\n", t2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("child thread is finished...\n");
}

void func3()
{
    DIR* dirp = opendir(".");

    if( dirp != NULL )
    {
        struct dirent* dp = NULL;

        while( (dp = readdir(dirp)) != NULL )
        {
            struct stat sb = {0};

            if( stat(dp->d_name, &sb) != -1 )
            {
                printf("name: %s, type: %d, len: %ld, mtime: %s", 
                        dp->d_name, dp->d_type, sb.st_size, ctime(&sb.st_mtime));
            }
        }
    }

    closedir(dirp);
}

void file_copy(const char* dst, const char* src)
{
    int dfd = open(dst, O_WRONLY|O_CREAT, 0600);
    int sfd = open(src, O_RDONLY);

    if( (dfd != -1) && (sfd != -1) )
    {
        char buf[512] = {0};
        int len = 0;

        while( (len = read(sfd, buf, sizeof(buf))) > 0 )
        {
            write(dfd, buf, len);
        }

        close(dfd);
        close(sfd);
    }
}

int main()
{
    //file_copy("new.out", "a.out");
	func3();

    return 0;
}

