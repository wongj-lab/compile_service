#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include "page.h"
#include "response.h"

enum { TypeAll = 0x00, TypeDir = 0x04, TypeFile = 0x08 };

typedef struct
{
    const int length;
    RowInfo data[];
} FileEntry;

static char* GetAbsPath(const char* relative,const char* root)
{
	int reLen = strlen(relative);
	int rootLen = strlen(root);
	char* ret = malloc(reLen+rootLen+2);

	if( ret )
	{
		strcpy(ret,root);

		if( (relative[0] == '/')&&(ret[rootLen-1] == '/') )ret[rootLen-1]=0;
		if((relative[0]!='/')&&(ret[rootLen-1]!='/')) strcat(ret,"/");

		strcat(ret,relative);
	}

	return ret;
}

static int IsDotPath(const char* path)
{
	int ret = -1;

	if( path )
	{
		ret = (strcmp(path,".") == 0) || (strcmp(path,"..") == 0);
	}

	return ret;
}

static int GetEntryCount(const char* path)
{
	int ret = -1;
	DIR* dirp = opendir(path);

	if( dirp != NULL )
	{
		struct dirent* dp = NULL;

		ret = 0;
		
		while( (dp = readdir(dirp)) != NULL )
		{
			if(!IsDotPath(dp->d_name))
			{
				ret++;
			}
		}
	}
	closedir(dirp);

	return ret;
}

static void SortFileEntry(FileEntry* fe)
{
	RowInfo* temp = malloc(sizeof(*temp));

	if( fe && temp )
	{
		int i = 0;
		int j = 0;

		for(i=0;i<fe->length;i++)
		{
			int min = i;
			for(j=i;j<fe->length;j++)
			{
				if(strcmp(fe->data[min].name,fe->data[j].name) > 0)
				{
					min = j;
				}
			}

			*temp = fe->data[i];
			fe->data[i] = fe->data[min];
			fe->data[min] = *temp;
		}
	}

	free(temp);
}

static int MakeEntryItem(RowInfo* item,struct dirent* dp,const char*ap,const char* req)
{
	int ret = 0;
	char buf[32] = {0};
	struct stat sb = {0};
	char* path = GetAbsPath(dp->d_name,ap);

	if( path && (ret = (stat(path,&sb))!=-1) )
	{
		strcpy(item->link,req);
		(strcmp(req,"/")!=0)?strcat(item->link,"/"):0;
		strcat(item->link,dp->d_name);

		strcpy(item->name,dp->d_name);

		if(dp->d_type == TypeFile)
		{
			strcpy(item->type,"File");

			if(sb.st_size < 1024)
			{
				sprintf(buf,"%ld",sb.st_size);
				strcpy(item->size,buf);
				strcat(item->size," Byte");
			}
			else if( (sb.st_size / 1024) < 1024 )
			{
				sprintf(buf,"%ld",sb.st_size / 1024);
				strcpy(item->size,buf);
				strcat(item->size," KB");
			}
			else
			{
				sprintf(buf,"%ld",sb.st_size / 1024 / 1024);
				strcpy(item->size,buf);
				strcat(item->size," MB");
			}
		}
		else
		{
			strcpy(item->type,"Folder");

			sprintf(buf,"%d",GetEntryCount(path));
			strcpy(item->size,buf);
			strcat(item->size," Item");
		}
	}

	free(path);

	return ret;
}

static FileEntry* GetEntry(const char* req,const char* root,int type)
{
	char* ap = GetAbsPath(req,root);
	DIR* dirp = NULL;
	FileEntry* ret = NULL;

	if( ap && (dirp = opendir(ap)) )
	{
		const int STEP = 5;
		struct dirent* dp = NULL;
		int max = 0;
		int* pLen = NULL;

		ret = malloc(sizeof(*ret));

		if( ret )
		{
			pLen = (int*)&ret->length;
			*pLen = 0;
		}

		while( pLen && ((dp = readdir(dirp)) != NULL) )
		{
			if(*pLen == max)
			{
				max = max + STEP;
				ret = realloc(ret,sizeof(*ret) + sizeof(RowInfo)*max);
				pLen = (int*)&ret->length;
			}

			if( ret && ((type == TypeAll) ||(type == dp->d_type)) )
			{
				if(!IsDotPath(dp->d_name) && MakeEntryItem(&ret->data[*pLen],dp,ap,req))
				{
					*pLen = *pLen + 1;
				}
			}
		}

		SortFileEntry(ret);
	}

	free(ap);
	closedir(dirp);

	return ret;
}

static char* MakeHTML(const char* req,const char* root)
{
	char* ret = NULL;
	Table* table = CreateTable();

	if( table )
	{
		FileEntry* fe = NULL;
		char* ts = NULL;
		char* resp = NULL;
		RowInfo* back = NULL;
		int i = 0;

		if( ((strcmp(req,"/"))!=0) && (back = calloc(1,sizeof(*back))))
		{
			i = strlen(req) -1;

			strcpy(back->link,req);

			while(back->link[i] != '/')i--;

			i?(back->link[i] = 0):(back->link[i+1] = 0);

			strcpy(back->name,"./..");
			strcpy(back->type,"Back..");

			table = InsertRow(table,back);
		}

		free(back);

		fe = GetEntry(req,root,TypeDir);

		for(i=0;fe && (i<fe->length);i++)
		{
			table = InsertRow(table,&fe->data[i]);
		}

		free(fe);

		fe = GetEntry(req,root,TypeFile);

		for(i=0;fe && (i<fe->length);i++)
		{
			table = InsertRow(table,&fe->data[i]);
		}

		free(fe);

		ts = ToTableString(table);

		ret = ts ? ToPageString(req,ts): NULL;

		free(ts);

	}

	FreeTable(table);

	return ret;
}

static int Response(TcpClient* client,const char* html)
{
	const char* HTTP_FORMAT =    "HTTP/1.1 200 OK\r\n"
                                 "Server:Test Http Server\r\n"
                                 "Content-Length:%d\r\n"
                                 "Content-Type:text/html\r\n"
                                 "Connection:close\r\n\r\n"
                                 "%s";

	int ret = 0;

	if( html )
	{
		char* resp = malloc(strlen(HTTP_FORMAT) + strlen(html) + 16);

		if( resp )
		{
			sprintf(resp,HTTP_FORMAT,strlen(html),html);

			ret = (TcpClient_SendRaw(client,resp,strlen(resp)) > 0);
		}

		free(resp);
	}

	return ret;
}

static int DirReqHandler(TcpClient* client,const char* req,const char* root)
{
	char* html = MakeHTML(req,root);
	int ret = Response(client,html);

	free(html);
}

static int BadReqHandler(TcpClient* client,const char* req,const char* root)
{
	char* html = ToErrString(req);
	int ret = Response(client,html);

	free(html);
}

static int FileReqHandler(TcpClient* client,const char* req,const char* root)
{
	const char* HTTP_FORMAT =    "HTTP/1.1 200 OK\r\n"
                                 "Server:Test Http Server\r\n"
                                 "Content-Length:%d\r\n"
                                 "Content-Type:application/*\r\n"
                                 "Connection:close\r\n\r\n";

	const int BUF_SIZE = 1024;

	int ret = 0;
	char* ap = GetAbsPath(req,root);
	int fd = open(ap,O_RDONLY);
	char* head = malloc(strlen(HTTP_FORMAT) + 32);
	char* buf = malloc(BUF_SIZE);

	if( ap && head && buf && (fd != -1) )
	{
		int max = lseek(fd,0,SEEK_END);
		int len = 0;

		sprintf(head,HTTP_FORMAT,max);

		len = TcpClient_SendRaw(client,head,strlen(head));

		lseek(fd,0,SEEK_SET);

		while( (len>0)&&((len = read(fd,buf,BUF_SIZE)) >0) )
		{
			ret += TcpClient_SendRaw(client,buf,len);
		}

		ret = (ret == max);
	}

	free(buf);
	free(head);
	free(ap);
	close(fd);

	return ret;
}


int RequestHandler(TcpClient* client,const char* req,const char* root)
{
	int ret = 0;
	if( client && req && root )
	{
		char* ap = GetAbsPath(req,root);

		if( ap && (access(ap,F_OK) == 0) )
		{
			if(GetEntryCount(ap)<0)
			{
				ret = FileReqHandler(client,req,root);
			}
			else
			{
				ret = DirReqHandler(client,req,root);
			}
		}
		else
		{
			ret = BadReqHandler(client,req,root);
		}
		free(ap);
	}

	return ret;
}
