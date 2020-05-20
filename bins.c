#include "bins.h"



//list all files in a dir
void my_ls(DirectoryHandle* dir){
	//checking
	assert(dir != 0 && "[my_ls] Dir handler not valid.\n");

	printf("Printing content of %s\n", dir->dcb->fcb.name);
	printf("Name\t\t\tDir\n");

	int i=0, entries = 0;
	//check the first block
	while(entries < dir->dcb->num_entries){	//dir->dcb->num_entries
		if(!dir->dcb->file_blocks[i]){
			i++;
			continue;
		}
		FirstFileBlock* ffb = (FirstFileBlock*) DiskDriver_readBlock(dir->sfs->disk, dir->dcb->file_blocks[i], 0);
		printf("%s\t\t\t%d\n", ffb->fcb.name, ffb->fcb.is_dir);
		i++;
		entries++;
	}

	int next = dir->dcb->header.next_block;
	if(next == -1)
		return;

	//if we're here, the dir is composed of more blocks, check them out!
	DirectoryBlock* db = (DirectoryBlock*) DiskDriver_readBlock(dir->sfs->disk, next, 0);
	i=0;
	entries = 0;
	while(next != -1){		//the same cycle we used before, adapted with right types
		while(entries < db->num_entries ){
			if(!db->file_blocks[i]){
				i++;
				continue;
			}
			FirstFileBlock* ffb = (FirstFileBlock*) DiskDriver_readBlock(dir->sfs->disk, db->file_blocks[i], 0);
			printf("%s\t%d\n", ffb->fcb.name, ffb->fcb.is_dir);
			i++;
			entries++;
		}
		next = db->header.next_block;
	}
}
