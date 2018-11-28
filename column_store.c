#include "BF.h"
#include <stdio.h>
#include <string.h>
#include "assistingFunctions.h"




int CS_CreateFiles(char **fieldNames, char *dbDirectory) 
{
	//Η CreateFile δημιουργεί 6 ξεχωριστά αρχεία για την αποθήκευση των στηλών.Ο κώδικας βρίσκεται μέσα στην επανάληψη και το μόνο
	//που αλλάζει είναι το path για κάθε αρχείο;γι'αυτό και τα if
	//printf("In CS_CreateFiles\n");
	BF_Init();
	int apotel,infoLength,fileDesc,i;
	char createpath[PATHLENGTH];
	for(i=0;i<NUMBEROFFILES;++i)
	{
		strcpy(createpath,dbDirectory);
		strcat(createpath,"/");
		if(strcmp(fieldNames[i],"id")==0)
		{
			strcat(createpath,"CSFile_id");
			apotel=BF_CreateFile(createpath);
			infoLength=sizeof(int);//It is an integer
		}
		else if (strcmp(fieldNames[i],"name")==0)
		{
			strcat(createpath,"CSFile_name");
			apotel=BF_CreateFile(createpath);
			infoLength=15;
		}
		else if (strcmp(fieldNames[i],"surname")==0)
		{
			strcat(createpath,"CSFile_surname");
			apotel=BF_CreateFile(createpath);
			infoLength=20;
		}
		else if (strcmp(fieldNames[i],"status")==0)
		{
			strcat(createpath,"CSFile_status");
			apotel=BF_CreateFile(createpath);
			infoLength=sizeof(char);
		}
		else if (strcmp(fieldNames[i],"dateOfBirth")==0)
		{
			strcat(createpath,"CSFile_dateOfBirth");
			apotel=BF_CreateFile(createpath);
			infoLength=11;
		}
		else if (strcmp(fieldNames[i],"salary")==0)
		{
			strcat(createpath,"CSFile_salary");
			apotel=BF_CreateFile(createpath);
			infoLength=sizeof(int);//It is an integer
		}
		//Μόλις δημιουργήθηκε το αρχείο,βάσει του ονόματος στήλης του τρέχοντος string κατά τη διάσχιση του πίνακα
		if(apotel<0)
		{//Κάτι πήγε στραβά με τη δημιουργία κάποιου αρχείου
			printf("Error\n");
			return -1;
		}
		else
		{//Τώρα μένει να γίνει αρχικοποίηση αυτού του αρχείου,την οποία αναλαμβάνει η συνάρτηση allocateInitialBlockForFile(args)
			//printf("Now working for file %s\n",createpath );
			fileDesc=BF_OpenFile(createpath);
			if(fileDesc<0)
			{
				printf("Error while opening file\n");
				return -1;
			}
			//printf("infoLength=%d\n",infoLength );
			allocateInitialBlockForFile(fileDesc,infoLength);
			if(BF_CloseFile(fileDesc)<0)
			{
				printf("Error closing file\n");
				return -1;
			}
		}
	}
  return 0;
}



//*********************************************************************************************************************



int CS_OpenFile(HeaderInfo* header_info, char *dbDirectory)
 {
  // Add your code here
 	//Εδώ χρειαζόμαστε 2 paths.Ένα για να διαβάσουμε το αρχείο headerInfo.txt,κι ένα για ν'ανοίξουμε κάθε CSFile
	char openpathF[PATHLENGTH];
	char openpathH[PATHLENGTH];
	strcpy(openpathH,dbDirectory);
	strcat(openpathH,"/headerInfo.txt");
	char filename[30];
	FILE *fp;
	//printf("My openpath is:%s\n",openpathH);
	fp=fopen(openpathH,"r");
	if(fp==NULL)
	{
		printf("Error while opening headerInfo file\n");
		return -1;
	}
	int i=0;
	while(!feof(fp))
	{
		if(i>=NUMBEROFFILES)
		{//Αν δεν μπει αυτή η break η διαδικασία θα γίνει μια επιπλέον φορά
			break;
		}
		fscanf(fp,"%s",filename);
		//printf("My filename is:%s\n",filename );
		strcpy(openpathF,dbDirectory);
		strcat(openpathF,"/");
		strcat(openpathF,filename);
		//printf("My open path is %s\n",openpathF);
		int fileDesc=BF_OpenFile(openpathF);
		if(fileDesc<0)
		{
			printf("Error during opening file\n");
			return -1;
		}
		//printf("FileDesc=%d\n",fileDesc );
		void* block;
		if(BF_ReadBlock(fileDesc,0,&block)<0)
		{
			printf("Error while reading block\n");
			return -1;
		}
		if(memcmp(block,"CSFile",strlen("CSFile"))!=0)
		{
			printf("This is not a CSFile!\n");
			return -1;
		}
		else
		{
			//printf("This is indeed a CSFile\n");
			header_info->column_fds[i].fd=fileDesc;//Store the headerInfo in our struct
			//^^Μόλις ενημερώθηκαν και οι πληροφορίες της δομής που κρατάει τις πληροφορίες σχετικά με τ'αρχεία
		}
		++i;
	}
	fclose(fp);
  return 0;
}



//*********************************************************************************************************************



int CS_CloseFile(HeaderInfo* header_info) 
{
	//printf("Going to close the CS files!\n");
	int i, myDesc;
	for (i = 0; i <NUMBEROFFILES; ++i)
	{
		//Παίρνουμε από τη δομή κάθε φορά τον κατάλληλο αναγνωριστικό αριθμό για το αντίστοιχο αρχείο.
		myDesc = header_info->column_fds[i].fd;
		//printf("Going to close CSFile : %d\n", myDesc);
		if (BF_CloseFile(myDesc) < 0)
		{
			BF_PrintError("Error while closing the heap file!");
			return -1;
		}
		/*else
		{
			//printf("File %d closed\n", myDesc);
		}*/
	}
	return 0;
}



//*********************************************************************************************************************



int CS_InsertEntry(HeaderInfo* header_info, Record record) 
{
  // Add your code here
	//printf("I got %d,%s,%s,%s,%s,%d\n",record.id,record.name,record.surname,record.dateOfBirth,record.status,record.salary );
	int recsalready,maxrecsnum,datalength,i,fileDesc,numofblocks,numofrecsintheblock,numofrecsinfile;
	void* block;
	void* blockstart;
	void* blockwrite;
	for(i=0;i<NUMBEROFFILES;++i)
	{
		fileDesc=header_info->column_fds[i].fd;
		if(BF_ReadBlock(fileDesc,0,&block)<0)
		{
			printf("Error reading block\n");
			return -1;
		}
		block=(char*)block+strlen("CSFile");
		memmove(&numofrecsinfile,block,sizeof(int));//Get the total number of recs in a file
		++numofrecsinfile;//Update it
		memmove(block,&numofrecsinfile,sizeof(int));
		block=(char*)block+sizeof(int);
		memmove(&datalength,block,sizeof(int));//Get the length of the data stored in this block
		block=(char*)block+sizeof(int);
		memmove(&maxrecsnum,block,sizeof(int));//Get the total number of entries that can be stored in a block of this file
		//printf("fileDesc=%d,maxrecsnum=%d,datalength=%d\n",fileDesc,maxrecsnum,datalength );
		numofblocks=BF_GetBlockCounter(fileDesc);
		if(numofblocks<0)
		{
			printf("Error getting block counter\n");
			return -1;
		}
		if(numofblocks==1)
		{//Αν έχουμε μόνο ένα μπλοκ είναι αυτό με τις αρχικές πληροφορίες,και πρέπει να δεσμεύσουμε ένα καινούργιο
			//printf("Theres only one block in this file so I will allocate another\n");
			if(BF_AllocateBlock(fileDesc)<0)
			{
				printf("Error allocating block counter\n");
				return -1;
			}
			numofblocks=BF_GetBlockCounter(fileDesc);
		}
		if((BF_ReadBlock(fileDesc,numofblocks-1,&blockstart)<0)||(BF_ReadBlock(fileDesc,numofblocks-1,&blockwrite)<0))
		{
			printf("Error reading block\n");
			return -1;
		}	
		memmove(&numofrecsintheblock,blockstart,sizeof(int));//Παίρνουμε τον αριθμό των εγγραφών που είναι ήδη στο μπλοκ
		if(numofrecsintheblock==maxrecsnum)//Έχουμε τόσες εγγραφές όσες χωράνε,άρα πρέπει να πάρουμε καινούργιο μπλοκ γι'αυτό το αρχείο
		{
			//printf("The previous block got filled so now I will allocate a new one\n");
			if(BF_AllocateBlock(fileDesc)<0)
			{
				printf("Error allocating block counter\n");
				return -1;
			}
			numofblocks=BF_GetBlockCounter(fileDesc);
			numofrecsintheblock=0;//Δεν υπάρχουν ακόμα εγγραφές σ'αυτό το μπλοκ
			if((BF_ReadBlock(fileDesc,numofblocks-1,&blockstart)<0)||(BF_ReadBlock(fileDesc,numofblocks-1,&blockwrite)<0))
			{
				printf("Error reading block\n");
				return -1;
			}	
			//^^Διαβάσαμε την αρχή του
		}
		//Τώρα,είτε έχουμε δεσμεύσει καινούργιο μπλοκ,είτε όχι,είμαστε ετοιμοι να γράψουμε
		blockwrite=(char*)blockwrite+sizeof(int)+numofrecsintheblock*datalength;
		if(fileDesc==0)
		{
			memmove(blockwrite,&record.id,datalength);
		}
		else if (fileDesc==1)
		{
			memmove(blockwrite,record.name,datalength);	
		}
		else if (fileDesc==2)
		{
			memmove(blockwrite,record.surname,datalength);	
		}
		else if (fileDesc==3)
		{
			memmove(blockwrite,record.status,datalength);	
		}
		else if (fileDesc==4)
		{
			memmove(blockwrite,record.dateOfBirth,datalength);	
		}
		else if (fileDesc==5)
		{
			memmove(blockwrite,&record.salary,datalength);	
		}
		++numofrecsintheblock;
		memmove(blockstart,&numofrecsintheblock,sizeof(int));//Update the total number of records in the block
		if(BF_WriteBlock(fileDesc,numofblocks-1)<0)
		{
			printf("Error writing block\n");
			return -1;
		}
	}
  return 0;
} 



//*********************************************************************************************************************



void CS_GetAllEntries(HeaderInfo* header_info, char *fieldName, void *value,char **fieldNames, int n)
{
	//printf("Going to decide how to print\n");
	if(value==NULL)
	{
		PrintAllRecords(header_info);
	}
	else
	{
		int i;
		for(i=0;i<NUMBEROFFILES;++i)
		{
			if(strcmp(fieldName,fieldNames[i])==0)
			{
				//printf("Going to print the entries\n");
				PrintAFile(header_info->column_fds[i].fd,value,fieldName,fieldNames);
				//printf("Okay,printed and returning\n");
				break;
			}
		}
	}
}

