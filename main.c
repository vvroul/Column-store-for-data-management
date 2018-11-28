#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "column_store.h"



#define dbDirectory "/home/users1/sdi1300025/dbDir"
//sample directory path: "/path/dbDir"


int main(void) {
  /* Καλείστε να γράψετε κώδικα που να αποδεικνύει την ορθότητα του προγράμματός σας */
  // Add your test code here
	//Αρχικά φτιάχνουμε και αρχικοποιούμε τη δομή headerinfo η οποία θα "κρατάει" χρήσιμες πληροφορίες σχετικά με τ'αρχεία
	//Αυτή η δομή αποτελείται από έναν πίνακα με τόσες θέσεις όσες και τ'αρχεία.Σε κάθε θέση αποθηκεύεται τ'όνομα του αρχείου,καθώς και το αναγνωριστικό ανοίγματός του.
	ColumnFds columnfds[NUMBEROFFILES];
	char* fieldnames[NUMBEROFFILES];
	fieldnames[0]="id";
	fieldnames[1]="name";
	fieldnames[2]="surname";
	fieldnames[3]="status";
	fieldnames[4]="dateOfBirth";
	fieldnames[5]="salary";
	//Μόλις δημιουργήθηκε ο πίνακας των πεδίων
	int boxes,i;
	for (i = 0; i <NUMBEROFFILES; ++i)
	{
		boxes=strlen(fieldnames[i])+1;
		columnfds[i].columnName=malloc(boxes);
		strcpy(columnfds[i].columnName,fieldnames[i]);
	}
	//printf("Initialised and going to print\n");
	HeaderInfo headerinfo;
	headerinfo.column_fds = columnfds;
	//Εφεξής η αναφορά στον πίνακα θα γίνεται με τ'όνομα column_fds,αυτό δηλαδή που δηλώνεται μέσα στο struct
	/*for(i=0;i<NUMBEROFFILES;++i)
	{
		//printf("...printing...\n");
		printf("%s\n",headerinfo.column_fds[i].columnName );
	}*/
	//Η δομή είναι έτοιμη
	//Τώρα θα δημιουργήσουμε το directory του προγράμματος 
	//Creation of a directory
	struct  stat st={0};
	if (stat(dbDirectory, &st) == -1)
	{
		//printf("Going to create the directory\n");
    	int apotel=mkdir(dbDirectory, 0700);
    	if(apotel==0)
    	{
    		printf("Directory created successfully %d\n",apotel);
    	}
    	else
    	{
    		printf("%s\n",strerror(errno));//ERROR
    	}
	}
	else
	{
		printf("The directory already exists\n");
	}
	//Στη συνέχεια θα δημιουργήσουμε σε αυτό το αρχείο headerinfo.txt και θα γράψουμε τα ονόματα των αρχείων
	FILE *fp;
	fp=fopen("./dbDir/headerInfo.txt","w");
	if(fp!=NULL)
	{
		fprintf(fp, "CSFile_id\n");
		fprintf(fp, "CSFile_name\n");
		fprintf(fp, "CSFile_surname\n");
		fprintf(fp, "CSFile_status\n");
		fprintf(fp, "CSFile_dateOfBirth\n");
		fprintf(fp, "CSFile_salary\n");
	}
	else
	{
		printf("Cannot write in file\n");
	}
	fclose(fp);
	//printf("Okay,now to create the CS files\n");
	//Τώρα πάμε να δημιουργήσουμε τ'αρχεία αποθήκευσης στηλών
	if(CS_CreateFiles(fieldnames,dbDirectory)<0)
	{
		printf("Error during file creation\n");
	}
	else
	{
		//printf("Files created OK\n");
	}
	//printf("Okay,now going to open them\n");
	//Τώρα πάμε ν'ανοίξουμε τα εν λόγω αρχεία
	if(CS_OpenFile(&headerinfo, dbDirectory)<0)
	{
		printf("Error opening files\n");
	}
	/*else
	{
	//	printf("Files opened OK\n");
		//printf("Lets print the fileDescs\n");
		for(i=0;i<NUMBEROFFILES;++i)
		{
			//printf("...printing...\n");
			printf("%d\n",headerinfo.column_fds[i].fd );
		}
	}*/
	//printf("Lets go fill the files\n");
	//Ανοίγουμε το αρχείο για να διαβάσουμε, και στη συνέχεια γράφουμε τις εγγραφές στα αρχεία μας
	FILE *recfp;
	recfp = fopen("records.txt", "r");
	Record myRecord;
	while (!feof(recfp))
	{
		fscanf(recfp, "%d", &myRecord.id);
		fscanf(recfp, "%s", myRecord.name);
		fscanf(recfp, "%s", myRecord.surname);
		fscanf(recfp, "%s", myRecord.status);
		fscanf(recfp, "%s", myRecord.dateOfBirth);
		fscanf(recfp, "%d", &myRecord.salary);

		CS_InsertEntry(&headerinfo, myRecord);
	//	printf("Records inserted successfully!\n");
	}
	//printf("Okay lets print them\n");
	//Εκτυπώνουμε τις εγγραφές που ζητούνται παρακάτω, στα κατάλληλα πεδία
	int testNum = 1193;
	char nameTest[15];
	strcpy(nameTest,"Mose");
	char surnameTest[20] = "Lowber";
	char myStatus[1] = "M";
	char dateTest[11] = "20-36-1985";
	int sNum = 5000;
	//Με τις κατάλληλες αλλαγές στο 2ο και 3ο όρισμα μπορείς να τεστάρεις τη συνάρτηση
	//π.χ. για 2ο αν βάλεις το "name", και 3ο το nameTest
	CS_GetAllEntries(&headerinfo, fieldnames[2], NULL, fieldnames, 6);
	//Κλείσιμο όλων των αρχείων
	if (CS_CloseFile(&headerinfo) < 0)
	{
		printf("Error closing files!\n");
	}
	/*else
	{
	//	printf("All files closed successfully!\n");
	}*/
	//Αποδέσμευση των πόρων της δομής,πριν τον τερματισμό του προγράμματος
	for(i=0;i<NUMBEROFFILES;++i)
	{
		//printf("..freeing...\n");
		free(headerinfo.column_fds[i].columnName);
	}
	//printf("GOODBYE\n");
}
