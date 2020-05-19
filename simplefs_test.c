#include "simplefs.h"
#include <stdio.h>
#include <stdlib.h>
#include "bins.h"



int main(int agc, char** argv) {
	DiskDriver* disk0 = malloc(sizeof(DiskDriver));
	SimpleFS* sfs = malloc(sizeof(SimpleFS));

	

	//start the test with same disk
	DiskDriver_open(disk0, "disk0.dat", 1024);

	//start the test formatting everything
	//SimpleFS_format(sfs, disk0, "disk0.dat", 1024);



	DirectoryHandle* rh = SimpleFS_open(sfs, disk0);
	my_ls(rh);
	DirectoryHandle* fh = SimpleFS_openDir(rh, "fagiolo");
	if(fh != 0)
		my_ls(fh);


//==============
	//2- create size files
	int size = 3;

	FileHandle* f[size];
	int i;
	for(i=0; i<size; i++){
		char name [10];
		sprintf(name, "f%d.txt", i+3);
		name[8]=0;
		SimpleFS_createFile(rh, name);
		f[i] = SimpleFS_openFile(rh, name);
	}
	SimpleFS_printFileData(f[0]);


	//3- mkdir
	SimpleFS_mkDir(rh, "fagiolo");
	fh = SimpleFS_openDir(rh, "fagiolo");
	printf("Created dir named: %s\n", fh->dcb->fcb.name);

	//4- ls("/");
	my_ls(rh);

	int size2 = 3;

	//add files to fagiolo
	FileHandle* g[size2];
	for(i=0; i<size2; i++){
		char name [10];
		sprintf(name, "g%d.txt", i+10);
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

//=================
	//printing dir's blocks
	SimpleFS_printDirData(rh);
	SimpleFS_printDirData(fh);

	//close dir


	SimpleFS_closedir(rh);
	SimpleFS_closedir(fh);


	//=============================
	free(sfs);
	DiskDriver_close(disk0, 0);
	free(disk0);

	return 0;
}

