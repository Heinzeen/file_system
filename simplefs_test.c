#include "simplefs.h"
#include <stdio.h>
#include <stdlib.h>



int main(int agc, char** argv) {
	DiskDriver* disk0 = malloc(sizeof(DiskDriver));
	SimpleFS* sfs = malloc(sizeof(SimpleFS));

	//==============================
	//restore the disk
	DiskDriver_init(disk0, "disk0.dat", 1024);
	DiskDriver_close(disk0, 1);

	DiskDriver_open(disk0, "disk0.dat", 1024);

	SimpleFS_init(sfs, disk0);
	
	DirectoryHandle* rh = SimpleFS_open(sfs, disk0);

	printf("Root name: %s\n", rh->dcb->fcb.name);
	printf("Current number of elements %d\n", rh->dcb->num_entries);

	printf("creating file in root\n");
	FileHandle* f0 = SimpleFS_createFile(rh, "f0.txt");

	//checking
	printf("file created in: %s\n", f0->directory->fcb.name);
	printf("Current number of elements %d\n", rh->dcb->num_entries);

	printf("creating file in root\n");
	FileHandle* f1 = SimpleFS_createFile(rh, "f1.txt");

	//checking
	if(f1!=0){
		printf("file created in: %s\n", f1->directory->fcb.name);
		printf("Current number of elements %d\n", rh->dcb->num_entries);
	}

	printf("creating file in root\n");
	FileHandle* f2 = SimpleFS_createFile(rh, "f1.txt");

	//checking
	if(f2!=0){
		printf("file created in: %s\n", f2->directory->fcb.name);
		printf("Current number of elements %d\n", rh->dcb->num_entries);
	}
	DiskDriver_close(disk0, 0);

	free(rh);
	free(f0);
	//=============================
	free(sfs);
	free(disk0);
	return 0;
}

/*
	DiskDriver* disk0 = malloc(sizeof(DiskDriver));
	
	//==============================
	//restore the disk
	DiskDriver_init(disk0, "disk0.dat", 1024);
	DiskDriver_close(disk0, 1);


	//3 write on a disk
	DiskDriver_open(disk0, "disk0.dat", 1024);
//1
	char temp[BLOCK_SIZE];
	int i;
	int b1;
	for(i=1; i<16; i++){
		memset(temp, i%256, BLOCK_SIZE);

		b1 = DiskDriver_getFreeBlock(disk0, 0);
		if( b1 == -1)
			printf("No buono\n");
		DiskDriver_writeBlock(disk0, temp, b1, BLOCK_SIZE, 0);
	}



//free
	int res = DiskDriver_freeBlock(disk0, b1);
	if( res == -1)
		printf("No buono\n");
	
	DiskDriver_close(disk0, 0);


	//=============================




	free(disk0);

	return 0;
*/
