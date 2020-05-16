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

	DiskDriver_init(disk0, "disk0.dat", 1024);

	DiskDriver_close(disk0);

	free(disk0);

	return 0;
}
