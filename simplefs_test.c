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

	//1 create a new disk
	DiskDriver* disk0 = malloc(sizeof(DiskDriver));

	DiskDriver_init(disk0, "disk0.dat", 1024);

	DiskDriver_close(disk0, 1);

	//2 open an existing disk
	DiskDriver_open(disk0, "disk0.dat", 1024);

	DiskDriver_close(disk0, 0);

	//3 write on a disk
	DiskDriver_open(disk0, "disk0.dat", 1024);
	char temp[BLOCK_SIZE];
	memset(temp, 4, BLOCK_SIZE);
	DiskDriver_writeBlock(disk0, temp, 1);

	memset(temp, 5, BLOCK_SIZE);
	DiskDriver_writeBlock(disk0, temp, 2);


	memset(temp, 1, BLOCK_SIZE);
	DiskDriver_writeBlock(disk0, temp, 4);
	DiskDriver_close(disk0, 0);

	//4 read from a disk
	DiskDriver_open(disk0, "disk0.dat", 1024);
	char* temp2 = DiskDriver_readBlock(disk0, 4);
	int i;
	for(i=0; i<BLOCK_SIZE; i++)
		printf("%d ", temp2[i]);
	printf("\n");
	DiskDriver_close(disk0, 0);


	free(disk0);

	return 0;
}
