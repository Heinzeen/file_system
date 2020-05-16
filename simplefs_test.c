#include "simplefs.h"
#include <stdio.h>
#include <stdlib.h>



int main(int agc, char** argv) {
/*
	printf("FirstBlock size %ld\n", sizeof(FirstFileBlock));
	printf("DataBlock size %ld\n", sizeof(FileBlock));
	printf("FirstDirectoryBlock size %ld\n", sizeof(FirstDirectoryBlock));
	printf("DirectoryBlock size %ld\n", sizeof(DirectoryBlock));
*/

	DiskDriver* disk0 = malloc(sizeof(DiskDriver));

	//restore the disk
	//DiskDriver_init(disk0, "disk0.dat", 1024);
	//DiskDriver_close(disk0, 1);


	//3 write on a disk
	DiskDriver_open(disk0, "disk0.dat", 1024);

	char temp[BLOCK_SIZE];
	memset(temp, 0xaa, BLOCK_SIZE);

	int b1 = DiskDriver_getFreeBlock(disk0, 0);
	if( b1 == -1)
		printf("No buono\n");

	DiskDriver_writeBlock(disk0, temp, b1);

	int b2 = DiskDriver_getFreeBlock(disk0, 0);
	if( b2 == -1)
		printf("No buono\n");

	DiskDriver_writeBlock(disk0, temp, b2);

	int res = DiskDriver_freeBlock(disk0, b2);
	if( res == -1)
		printf("No buono\n");
	



	DiskDriver_close(disk0, 0);

	//4 read from a disk
	DiskDriver_open(disk0, "disk0.dat", 1024);
	char* temp2 = DiskDriver_readBlock(disk0, b1);
	int i;
	for(i=0; i<BLOCK_SIZE; i++)
		printf("%d ", temp2[i]);
	printf("\n");
	DiskDriver_close(disk0, 0);


	free(disk0);

	return 0;
}
