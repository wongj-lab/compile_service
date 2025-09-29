#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "page.h"

static const char* PAGE_FORMAT = "<!DOCTYPE html>"
                                 "<html>"
                                 "  <head>"
                                 "    <meta charset=\"utf-8\">"
                                 "    <title>D.T.Software</title>"
                                 "  </head>"
                                 "  <body>"
                                 "    <h1>DT4SW Http File Server</h1>"
                                 "    <hr/>"
                                 "    <h3>Path: %s</h3>"
                                 "    %s"
                                 "  </body>"
                                 "</html>";

static const char* TABLE_BEGIN = "<table border=\"1\" width=\"100%\">";
                                                                  
static const char* TABLE_TITLE = "  <tr>"
                                 "    <th>File Name</th><th>File Type</th><th>File Size</th><th>Modify Time</th>"
                                 "  </tr>";
                                
static const char* ROW_FORMAT =  "<tr>"
                                 "  <td><a href=\"%s\">%s</a></td><td>%s</td><td>%s</td><td>%s</td>"
                                 "</tr>";     
                                
static const char* TABLE_END =   "  </tr>"
                                 "</table>";        

char* ToPageString(const char* path,const char* ts)
{
	char* ret = NULL;

	if( path && ts && (ret = malloc(strlen(PAGE_FORMAT)+strlen(path)+strlen(ts)+1)) )
	{
		sprintf(ret,PAGE_FORMAT,path,ts);
	}
	return ret;
}

Table* CreateTable()
{
	const int TITLE_LEN = strlen(TABLE_TITLE);
	char* ret = malloc(TITLE_LEN+1);

	if( ret )
	{
		strcpy(ret,TABLE_TITLE);
	}

	return ret;
}

char* ToTableString(Table* table)
{
	const int BEGIN_LEN = strlen(TABLE_BEGIN);
	const int END_LEN = strlen(TABLE_END);
	char* ret = NULL;

	if( table && (ret = malloc(strlen(table) + BEGIN_LEN + END_LEN + 1)) )
	{
		strcpy(ret,TABLE_BEGIN);
		strcpy(ret+BEGIN_LEN,table);
		strcpy(ret+BEGIN_LEN+strlen(table),TABLE_END);
	}

	return ret;
}

Table* InsertRow(Table* table,RowInfo* info)
{
	char* ret = NULL;
	if( table && info )
	{
		char* t = table;
		int len = strlen(t);
		char* buf = malloc(strlen(ROW_FORMAT) + sizeof(*info));

		if( buf )
		{
			sprintf(buf,ROW_FORMAT,info->link,info->name,info->type,info->size,info->time);

			ret = realloc(t,len+strlen(buf)+1);
			ret = ret?(strcpy(ret+len,buf),ret):t;
		}
		else
		{
			ret = t;
		}

		free(buf);
	}

	return ret;
}

void FreeTable(Table* table)
{
	free(table);
}
