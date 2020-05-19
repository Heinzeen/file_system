#include "simplefs.h"
#include <stdio.h>
#include <stdlib.h>
#include "bins.h"



int main(int agc, char** argv) {
	DiskDriver* disk0 = malloc(sizeof(DiskDriver));


	//==============================
	//restore the disk
	DiskDriver_init(disk0, "disk0.dat", 1024);
	DiskDriver_close(disk0, 1);

	//start the test
	DiskDriver_open(disk0, "disk0.dat", 1024);
	SimpleFS* sfs = malloc(sizeof(SimpleFS));

	SimpleFS_init(sfs, disk0);
	
	DirectoryHandle* rh = SimpleFS_open(sfs, disk0);
	my_ls(rh);




	//2- create size files
	int size = 2;

	FileHandle* f[size];
	int i;
	for(i=0; i<size; i++){
		char name [10];
		sprintf(name, "f%d.txt", i);
		name[8]=0;
		SimpleFS_createFile(rh, name);
		f[i] = SimpleFS_openFile(rh, name);
	}
	printf("Pointer= %p\n", f[0]);
		fflush(stdout);
	SimpleFS_printFileData(f[0]);


	//3- mkdir
	SimpleFS_mkDir(rh, "fagiolo");
	DirectoryHandle* fh = SimpleFS_openDir(rh, "fagiolo");
	printf("Created dir named: %s\n", fh->dcb->fcb.name);

	//4- ls("/");
	my_ls(rh);

	int size2 = 100;

	//add files to fagiolo
	FileHandle* g[size2];
	for(i=0; i<size2; i++){
		char name [10];
		sprintf(name, "g%d.txt", i);
		name[8]=0;
		SimpleFS_createFile(fh, name);
		g[i] = SimpleFS_openFile(fh, name);
	}

	//ls "fagiolo"
	my_ls(fh);


	//close files
	for(i=0; i<size; i++){
		SimpleFS_close(f[i]);
	}

	for(i=0; i<size2; i++){
		SimpleFS_close(g[i]);
	}


	//printing dir's blocks
	SimpleFS_printDirData(rh);
	SimpleFS_printDirData(fh);

	//close dir
	SimpleFS_closedir(fh);


	SimpleFS_closedir(rh);


	//=============================
	free(sfs);
	DiskDriver_close(disk0, 0);
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
