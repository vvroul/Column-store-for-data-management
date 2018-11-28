#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "assistingFunctions.h"



int allocateInitialBlockForFile(int fileDesc,int infoLength)
//This function allocates the first block of a CSFile and initialises it
{
	//Η συνάρτηση αυτή αναλαμβάνει την αρχικοποίηση του αρχείου.Για το σκοπό αυτό λαμβάνει ως ορίσματα το αναγνωριστικό ανοίγματος ενός αρχείου,καθώς και
	//το μήκος σε bytes του τύπου δεδομένων που φυλάσσεται σε αυτό το αρχείο.Αρχικά δεσμέυεται το 1ο μπλοκ του αρχείου,στο οποίο θα εγγραφούν οι αναγνωριστικές 
	//πληροφορίες ως εξής:
	//αρχικά γράφεται στο μπλοκ η συμβολοσειρά "CSFile",που υποδεικνύει ότι πρόκειται γι'αρχείο column_store
	//στη συνέχεια γράφεται ο αριθμός 0,καθώς δεν υπάρχουν αρχικά εγγραφές για το συγκεκριμένο αρχείο.Κάθε φορά που θα εχουμε μια καινούργια εγγραφή,αυτός
	//ο αριθμός αυξάνεται κατά 1
	//ο επόμενος αριθμός που εγγράφεται είναι το μήκος του τύπου δεδομένων που φυλάσσει το εν λόγω αρχείο
	//printf("I'm ready to allocate a new block for my file\n");
	if(BF_AllocateBlock(fileDesc)<0)//Error case;cannot allocate block
	{
		printf("Error during block allocation\n");
		return -1;
	}
	else//Block allocated fine
	{
		int numofblocks=BF_GetBlockCounter(fileDesc);
		if(numofblocks<0)//Error in the number of blocks
		{
			printf("Error during getting total number of blocks\n");
			return -1;	
		}
		else//Acquired number of blocks
		{
			//printf("The total number of blocks in this file is %d\n",numofblocks );
			void* block;
			if(BF_ReadBlock(fileDesc,numofblocks-1,&block)<0)//Error;couldn't read blocks
			{
				printf("Error during reading block\n");
				return -1;
			}
			else
			{
				//printf("Ok time to initilise the block\n");
				memmove(block,"CSFile",strlen("CSFile"));//CHECK FOR BUG
				//^^Indicating that this is a CSFile block
				block=(char*)block+strlen("CSFile");
				int numberofrecords=0;
				memmove(block,&numberofrecords,sizeof(int));
				//^^Initially there are no records in the file
				block=(char*)block+sizeof(int);
				memmove(block,&infoLength,sizeof(int));
				//^^The length of the record stored in this file
				int numofrecsinblock=BLOCK_SIZE/infoLength-sizeof(int);
				block=(char*)block+sizeof(int);
				//printf("numofrecsinblock=%d\n", numofrecsinblock);
				memmove(block,&numofrecsinblock,sizeof(int));
				//Done with the initialisation,going to write in the block
				if(BF_WriteBlock(fileDesc,numofblocks-1)<0)//Error case;cannot write on the block
				{
					printf("Error during writing in the block\n");
					return -1;
				}
				else
				{
					//printf("Seems like everything went fine,now going back\n");
					return 0;
				}
			}
		}
	}
}





void PrintAllRecords(HeaderInfo* header_info)
{
	int recsinblock,datalength,totalrecs,maxrecsnum,fileDesc;
	void* block;
	void* blockprint;
	int id,salary,i,j;
	char name[15],surname[20],status[1],dateOfBirth[11];
	int printed[NUMBEROFFILES],printingblock[NUMBEROFFILES];
	for(i=0;i<NUMBEROFFILES;++i)
	{
		printed[i]=0;//Αρχικά δεν έχουμε εκτυπώσει τπτ
		printingblock[i]=1;//Πάντα ξεκινάμε τις εκτυπώσεις από το 2ο μπλοκ
	}
	if(BF_ReadBlock(0,0,&block)<0)
	{
		BF_PrintError("Error reading block\n");
		return ;
	}
	block=(char*)block+strlen("CSFile");
	memmove(&totalrecs,block,sizeof(int));//Get the total number of recs in a file
	for(j=0;j<totalrecs;++j)
	{
		for(i=0;i<NUMBEROFFILES;++i)
		{
			fileDesc=header_info->column_fds[i].fd;
			if(BF_ReadBlock(fileDesc,0,&block)<0)
			{
				printf("Error reading block \n");
				return ;
			}
			//Αρχικά διαβάζουμε το πρώτο μπλοκ ενός αρχείου για να πάρουμε πληροφορίες
			block=(char*)block+strlen("CSFile")+sizeof(int);
			memmove(&datalength,block,sizeof(int));//Get the length of the data stored in this block
			block=(char*)block+sizeof(int);
			memmove(&maxrecsnum,block,sizeof(int));//Get the total number of entries that can be stored in a block of this filename
			//printf("Got the initial info datalength=%d,maxrecsnum=%d\n",datalength,maxrecsnum );
			if(printed[i]==maxrecsnum)
			{
				//printf("Moving to the next block\n");
				printingblock[i]=printingblock[i]+1;//Αν έχουμε εκτυπώσει τόσες εγγραφές όσες χωράνε στο μπλοκ,τότε προχωράμε στο επόμενο
				printed[i]=0;
			}
			//printf("Going to read our printing block %d\n",printingblock[i] );
			if(BF_ReadBlock(fileDesc,printingblock[i],&blockprint)<0)
			{
				printf("Error reading block\n");
				return ;
			}
			blockprint=(char*)blockprint+sizeof(int);//Προσπερνάμε τον αρχικό αριθμό
			blockprint=(char*)blockprint+printed[i]*datalength;//Προσπερνάμε όσες εγγραφές έχουμε ήδη εκτυπώσει
			if(fileDesc==0)
			{
				memmove(&id,blockprint,datalength);
				printf("%d",id );
				printed[i]=printed[i]+1;
			}
			else if (fileDesc==1)
			{
				memmove(name,blockprint,datalength);
				printf(",%s",name );	
				printed[i]=printed[i]+1;
			}
			else if (fileDesc==2)
			{
				memmove(surname,blockprint,datalength);	
				printf(",%s",surname );
				printed[i]=printed[i]+1;
			}
			else if (fileDesc==3)
			{
				memmove(status,blockprint,datalength);	
				printf(",%s",status );
				printed[i]=printed[i]+1;
			}
			else if (fileDesc==4)
			{
				memmove(dateOfBirth,blockprint,datalength);	
				printf(",%s",dateOfBirth );
				printed[i]=printed[i]+1;
			}
			else if (fileDesc==5)
			{
				memmove(&salary,blockprint,datalength);	
				printf(",%d\n",salary );
				printed[i]=printed[i]+1;
			}
		}
	}
	printf("Read\n");
	for(i=0;i<NUMBEROFFILES;++i)
	{
		printf("%d\n", BF_GetBlockCounter(header_info->column_fds[i].fd));
	}
	printf("blocks\n");
}




void PrintAFile(int fileDesc,void* value,char* fieldname,char** fieldNames)
//Prints the entries of a file based on a value
{
	//printf("Okay,Im here to print the %s\n",fieldname );
	int recsinblock,datalength,totalrecs,maxrecsnum;
	void* block;
	void* blockprint;
	int id,salary,i,j,compared=0,printingblock=1;
	char name[15],surname[20],status[1],dateOfBirth[11];
	if(BF_ReadBlock(fileDesc,0,&block)<0)//Αρχικά διαβάζουμε το πρώτο πρώτο μπλοκ για να πάρουμε τις πληροφορίες που χρειαζόμαστε
	{
		BF_PrintError("Error reading block\n");
		return ;
	}
	//printf("Got the pointer to the start\n");
	block=(char*)block+strlen("CSFile");
	memmove(&totalrecs,block,sizeof(int));//Get the total number of recs in a file
	//printf("Got the total number of recs in a file\n");
	block=(char*)block+sizeof(int);
	memmove(&datalength,block,sizeof(int));//Get the length of the data stored in this block
	//printf("Got the length of the data stored in this block\n");
	block=(char*)block+sizeof(int);
	memmove(&maxrecsnum,block,sizeof(int));//Get the total number of entries that can be stored in a block of this filename
	//printf("Got the total number of entries that can be stored in a block of this filename\n");
	//printf("totalrecs=%d,maxrecsnum=%d,datalength=%d\n",totalrecs,maxrecsnum,datalength);
	for(j=0;j<totalrecs;++j)
	{
		//printf("compared=%d\n",compared);
		if(compared==maxrecsnum)
		{
			//printf("We've checked a whole block;time to move on\n");
			++printingblock;
			compared=0;
		}
		//printf("Going to read the block %d\n",printingblock );
		if(BF_ReadBlock(fileDesc,printingblock,&blockprint)<0)
		{
			printf("Error reading block\n");
			return;
		}
		blockprint=(char*)blockprint+sizeof(int);//Προσπερνάμε τον αριθμό που είναι γραμμένος στην αρχή
		blockprint=(char*)blockprint+compared*datalength;//Προσπερνάμε όσες εγγραφές του μπλοκ έχουμε ήδη εκτυπώσει
		if(strcmp(fieldname,fieldNames[0])==0)
		{//Print id
			memmove(&id,blockprint,datalength);
			int* idcomp=value;
			if(id==(*idcomp))
			{
				printf("Id=%d\n",id );
			}
		}
		else if(strcmp(fieldname,fieldNames[1])==0)
		{//Print name
			memmove(name,blockprint,datalength);
			if(strcmp(name,value)==0)
			{
				printf("Name=%s\n",name);
			}
		}
		else if(strcmp(fieldname,fieldNames[2])==0)
		{//Print surname
			memmove(surname,blockprint,datalength);
			if(strcmp(surname,value)==0)
			{
				printf("Surname=%s\n",surname );
			}
		}
		else if(strcmp(fieldname,fieldNames[3])==0)
		{//Print status
			
			if(memcmp(blockprint,value,datalength)==0)
			{
				memmove(status,blockprint,datalength);
				printf("Status=%s\n",status );
			}
		}
		else if(strcmp(fieldname,fieldNames[4])==0)
		{//Print dateofbirth
			memmove(dateOfBirth,blockprint,datalength);
			if(strcmp(dateOfBirth,value)==0)
			{
				printf("DateOfBirth=%s\n",dateOfBirth );
			}
		}
		else if(strcmp(fieldname,fieldNames[5])==0)
		{//Print nsalary
			memmove(&salary,blockprint,datalength);
			int* salcomp=value;
			if(salary==(*salcomp))
			{
				printf("Salary=%d\n",salary );
			}
		}
		++compared;
	}
	printf("Read %d blocks\n", BF_GetBlockCounter(fileDesc));
}
