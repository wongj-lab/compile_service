#ifndef PAGE_H
#define PAGE_H
typedef void Table;

typedef struct
{
	char link[2048];
	char name[255];
	char type[32];
	char size[32];
	char time[32];
}RowInfo;

char* ToPageString(const char* path,const char* ts);
Table* CreateTable();
Table* InsertRow(Table* table,RowInfo* info);
char* ToTableString(Table* table);
void FreeTable(Table* table);


#endif

