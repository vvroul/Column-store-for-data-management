#include "column_store.h"

void PrintAllRecords(HeaderInfo* header_info);
//This function prints all the records written in the column files

int allocateInitialBlockForFile(int fileDesc,int infoLength);
//This function allocates the first block of a CSFile and initialises it

void PrintAFile(int fileDesc,void* value,char* fieldname,char** fieldNames);