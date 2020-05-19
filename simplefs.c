#include "simplefs.h"
#include <stdio.h>



//TODO maybe we should not ret -1 but close the program
// initializes a file system on an already made disk
// returns -1 in error, 0 otherwise
int SimpleFS_init(SimpleFS* fs, DiskDriver* disk){
	//checking
	assert(fs && "[SimpleFS_init] Fs not valid.");
	assert(disk && "[SimpleFS_init] Disk not valid.");

	fs->disk = disk;

	//creating root directory
	BlockHeader bh = {.block_number = 0, .previous_block = -1, .next_block = -1, .block_in_file = 0};
	FileControlBlock fcb = {.first_block = 0, .directory_block = 0, .block_in_disk = 0, .name = "/", .size_in_bytes = 0, .size_in_blocks = 1, .is_dir = 1};

	FirstDirectoryBlock fdb = {.header = bh, .fcb = fcb, .num_entries = 0, .room = (BLOCK_SIZE
											-sizeof(BlockHeader)
											-sizeof(FileControlBlock)
											-2*sizeof(int))/sizeof(int)};		//TODO add the file_blocks
	
	//write it into the block 0
	int res = DiskDriver_writeBlock(disk, &fdb, 0, sizeof(FirstDirectoryBlock), 0);
	check_errors(res , -1, "[SimpleFS_init] Cannot write the block.");

	return 0;
}

//TODO check for integrity (disk)
int SimpleFS_check(SimpleFS* fs){


	return 0;

}


// open an already made filse system
// returns 0 if everything is fine, and if the fs is good
DirectoryHandle* SimpleFS_open(SimpleFS* fs, DiskDriver* disk){
	//checking
	assert(fs && "[SimpleFS_init] Fs not valid.");
	assert(disk && "[SimpleFS_init] Disk not valid.");

	fs->disk = disk;

	if(SimpleFS_check(fs))
		return 0;

	//create the root handle
	DirectoryHandle* rh = malloc(sizeof(DirectoryHandle));
	assert(rh && "[SimpleFS_init] Cannot allocate rh.");

	rh->sfs = fs;
	rh->dcb = (FirstDirectoryBlock*) DiskDriver_readBlock(disk, 0, 0);
	rh->directory = 0;
	rh->current_block = (BlockHeader*) rh->dcb;
	rh->pos_in_dir = 0;
	rh->pos_in_block = 0;

	
	return rh;
}


//prints all the elements about a file
void SimpleFS_printFileData(FileHandle* f){
	//checking
	assert(f && "[SimpleFS_printFileData] File Handler not valid.\n");

	printf("======Printing a file=====\n");
	printf("Name: %s.\n", f->fcb->fcb.name);
	printf("Directory: %s.\n", f->directory->fcb.name);
	printf("First block: %d.\n", f->fcb->fcb.first_block);
	printf("Next block: %d.\n", f->fcb->header.next_block);
	printf("Dir first block: %d.\n", f->directory->fcb.first_block);
	printf("Block in disk: %d.\n", f->fcb->fcb.block_in_disk);
	printf("Size: %d.\n", f->fcb->fcb.size_in_bytes);
	printf("Total blocks: %d.\n", f->fcb->fcb.size_in_blocks);
	printf("Block %d, prev= %d, next= %d.\n", f->fcb->header.block_number, f->fcb->header.previous_block, f->fcb->header.next_block);
	printf("==========================\n");

}

//prints all the elements about a dir, debugging purpose
void SimpleFS_printDirData(DirectoryHandle* d){
	//checking
	assert(d && "[SimpleFS_printDirData] Dir Handler not valid.\n");

	printf("======Printing a Directory=====\n");
	if(d->directory != 0)
		printf("Parent: %s.\n", d->directory->fcb.name);
	else
		printf("Seems like we are in root (or we have a problem).\n");
	printf("Printing all the blocks now:\n");
	printf("Block %d, prev= %d, next= %d, num_entries= %d, room= %d.\n", d->dcb->header.block_number, d->dcb->header.previous_block, d->dcb->header.next_block, d->dcb->num_entries, d->dcb->room);
	int next = d->dcb->header.next_block;
	while(next != -1){
		DirectoryBlock* bh = (DirectoryBlock*) DiskDriver_readBlock(d->sfs->disk, next, 0);
		printf("Block %d, prev= %d, next= %d, num_entries= %d, room= %d.\n", bh->header.block_number , bh->header.previous_block, bh->header.next_block, bh->num_entries, bh->room);
		next = bh->header.next_block;
	}
	printf("==========================\n");
}


// check for every dir's block if that name already exists
void* SimpleFS_checkname(DirectoryHandle* d, const char* filename){

	FirstFileBlock* ffb;		//the part that we're using of this structure is the same for dirs
					//so we don't need to change anything for them

	//the first block is different
	int n = d->dcb->num_entries, i;
	for(i=0; i<n; i++){
		ffb = (FirstFileBlock*) DiskDriver_readBlock(d->sfs->disk, d->dcb->file_blocks[i], 0);
		if(!strcmp(ffb->fcb.name, filename))
			return ffb;
	}

	//if there is more blocks, iterate through them
	int next = d->dcb->header.next_block;
	while(next != -1){
		DirectoryBlock * dirblock = (DirectoryBlock*) DiskDriver_readBlock(d->sfs->disk, next, 0);
		int n = dirblock->num_entries;
			for(i=0; i<n; i++){
			ffb = (FirstFileBlock*) DiskDriver_readBlock(d->sfs->disk, dirblock->file_blocks[i], 0);
			if(!strcmp(ffb->fcb.name, filename))
					return ffb;
		}
		next = dirblock->header.next_block;
	}

	return 0;
}

//TODO increment size_in_blocks

//int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num, int count, int block_offset){

int SimpleFS_addtodir(DirectoryHandle* d, int block){

	//check if we have space in this block
	if(d->dcb->room > 0){
		//we have to iterate through the array to find a free spot
		int spot = -1, i=0;
		while(spot != 0){
			spot = d->dcb->file_blocks[i];
			i++;
		}
		//debugging print
		char msg[64];
		sprintf(msg, "Adding block %d to dir in block %d (main), in spot %d.", block, d->dcb->fcb.first_block, i-1);
		debug_print(msg);
		d->dcb->num_entries += 1;
		d->dcb->room -= 1;
		d->dcb->file_blocks[i - 1] = block;			//here
		//save the things in memory
		DiskDriver_writeBlock(d->sfs->disk, d->dcb, d->dcb->fcb.first_block, sizeof(FirstDirectoryBlock), 0);
		return 0;
	}

	int next = d->dcb->header.next_block, prev = d->dcb->fcb.first_block;
	DirectoryBlock * db = 0;
	int cnt = 0;
	while(next != -1){
		prev = next;		//save the state of prev, in case we need later
		cnt += 1 ;
		db = (DirectoryBlock*) DiskDriver_readBlock(d->sfs->disk, next, 0);
		if(db->room > 0){
			//we have to iterate through the array to find a free spot
			int spot, i=0;
			while(spot != 0){
				spot = d->dcb->file_blocks[i];
				i++;
			}
			//debugging print
			char msg[64];
			sprintf(msg, "Adding block %d to dir in block %d, in spot %d.", block, next, i-1);
			debug_print(msg);
			db->num_entries += 1;
			db->room -= 1;
			db->file_blocks[i - 1] = block;			//here
			//save the things in memory
			DiskDriver_writeBlock(d->sfs->disk, db, next, sizeof(DirectoryBlock), 0);
			return 0;
		}
		next = db->header.next_block;
	}
	

	//create another block and add it to the list
	int new_block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	//debugging print
	char msg[64];
	sprintf(msg, "Adding block %d to dir in block %d, in spot 0.", block, new_block);
	debug_print(msg);

	if(db == 0)		//then we didn't iterate
		d->dcb->header.next_block = new_block;
		
	else
		db->header.next_block = new_block;

	//initialize the block
	BlockHeader header = {.block_number = new_block, .next_block = -1, .previous_block = prev, .block_in_file = cnt};
	int room = (BLOCK_SIZE-sizeof(BlockHeader)-2*sizeof(int))/sizeof(int) - 1 ;
	int entries = 1;

	DirectoryBlock new_db = {. header=header, .room=room, .num_entries = entries};
	new_db.file_blocks[entries - 1] = block;					//here
	//write on disk
	DiskDriver_writeBlock(d->sfs->disk, &new_db, new_block, sizeof(DirectoryBlock), 0);
	return 0;
}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
int SimpleFS_createFile(DirectoryHandle* d, const char* filename){
	//checking
	assert(d && "[SimpleFS_createFile] Dir not valid.");
	assert(filename && "[SimpleFS_createFile] Filename not valid.");

	//debugging prints
	debug_print("Creating new file.");
	debug_print((char*)filename);		//casted to avoid warning with the const


	//ask for a new block
	int block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	if(block == -1){
		printf("Cannot allocate a new block.\n");
		return 1;
	}

	//check if there is no other files with the same name
	if(SimpleFS_checkname(d, filename)){
		printf("A file with that name already exists!\n");
		return 1;
	}

	//create the block header
	BlockHeader bh = {.block_number = block, .previous_block = -1, .next_block = -1, .block_in_file = 0};

	//create fcb
	FileControlBlock fcb = {.first_block = block, .directory_block = d->dcb->fcb.first_block, .block_in_disk = block, .size_in_bytes = 0, .size_in_blocks = 1, .is_dir = 0};
	memcpy(fcb.name, filename, strlen(filename));

	FirstFileBlock ffb = {.header = bh, .fcb = fcb};

	//write on disk
	DiskDriver_writeBlock(d->sfs->disk, &ffb, block, sizeof(FirstFileBlock), 0);

	
	//add it to the directory
	int res = SimpleFS_addtodir(d, block);
	check_errors(res , -1, "[SimpleFS_createFile] Cannot add to dir.");

	//save the new things on the dir's block
	//res = DiskDriver_writeBlock(d->sfs->disk, d->dcb, d->dcb->fcb.first_block, sizeof(FirstDirectoryBlock), 0);
	check_errors(res , -1, "[SimpleFS_createFile] Cannot write the block.");

	return 0;
}


// opens a file in the	directory d. The file should be exisiting
FileHandle* SimpleFS_openFile(DirectoryHandle* d, const char* filename){
	//checking
	assert(d && "[SimpleFS_openFile] Dir not valid.");
	assert(filename && "[SimpleFS_openFile] Filename not valid.");

	FirstFileBlock* file = (FirstFileBlock*) SimpleFS_checkname(d, filename);

	if(!file)	//file not found
		return 0;

	//create the file handler
	FileHandle* fh = malloc(sizeof(FileHandle));
	fh->sfs = d->sfs;
	fh->fcb = (FirstFileBlock*)DiskDriver_readBlock(d->sfs->disk, file->fcb.first_block, 0);
	fh->directory = d->dcb;
	fh->current_block = (BlockHeader*)DiskDriver_readBlock(d->sfs->disk, file->fcb.first_block, 0);
	fh->pos_in_file = 0;

	return fh;


}

// creates a new directory in the current one (stored in fs->current_directory_block)
// 0 on success
// -1 on error
int SimpleFS_mkDir(DirectoryHandle* d, char* dirname){		//many parts are like createfile
	//checking
	assert(d && "[SimpleFS_mkDir] Dir not valid.");
	assert(dirname && "[SimpleFS_mkDir] Dirname not valid.");

	//debugging prints
	debug_print("Creating new directory.");
	debug_print((char*)dirname);		//casted to avoid warning with the const


	//ask for a new block
	int block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	if(block == -1){
		printf("Cannot allocate a new block.\n");
		return 1;
	}

	//check if there is no other files with the same name
	if(SimpleFS_checkname(d, dirname)){
		printf("A directory with that name already exists!\n");
		return 1;
	}

	//create the block header
	BlockHeader bh = {.block_number = block, .previous_block = -1, .next_block = -1, .block_in_file = 0};

	//create fcb
	FileControlBlock fcb = {.first_block = block, .directory_block = d->dcb->fcb.first_block, .block_in_disk = block, .size_in_bytes = 0, .size_in_blocks = 1, .is_dir = 1};
	memcpy(fcb.name, dirname, strlen(dirname));

	FirstDirectoryBlock fdb = {.header = bh, .fcb = fcb, .num_entries = 0, .room = (BLOCK_SIZE
											-sizeof(BlockHeader)
											-sizeof(FileControlBlock)
											-2*sizeof(int))/sizeof(int)};		//TODO add the file_block

	//write on disk
	DiskDriver_writeBlock(d->sfs->disk, &fdb, block, sizeof(FirstDirectoryBlock), 0);

	//add it to the directory
	int res = SimpleFS_addtodir(d, block);
	check_errors(res , -1, "[SimpleFS_mkDir] Cannot add to dir.");

	//save the new things on the dir's block
	res = DiskDriver_writeBlock(d->sfs->disk, d->dcb, 0, sizeof(FirstDirectoryBlock), 0);
	check_errors(res , -1, "[SimpleFS_mkDir] Cannot write the block.");

	return 0;

}

DirectoryHandle* SimpleFS_openDir(DirectoryHandle* d, const char* dirname){
	//checking
	assert(d && "[SimpleFS_openDir] Dir not valid.");
	assert(dirname && "[SimpleFS_openDir] Dirname not valid.");

	FirstDirectoryBlock* dir = (FirstDirectoryBlock*) SimpleFS_checkname(d, dirname);

	if(!dir)	//file not found
		return 0;


	//create the directory handler
	DirectoryHandle* dh = malloc(sizeof(FileHandle));
	dh->sfs = d->sfs;
	dh->dcb = (FirstDirectoryBlock*)DiskDriver_readBlock(d->sfs->disk, dir->fcb.first_block, 0);
	dh->directory = d->dcb;
	dh->current_block = (BlockHeader*)DiskDriver_readBlock(d->sfs->disk, dir->fcb.first_block, 0);
	dh->pos_in_dir = 0;
	dh->pos_in_block = 0;
	

	return dh;

}

// closes a file handle (destroyes it)
int SimpleFS_close(FileHandle* f){
	//checking
	assert(f != 0 && "[SimpleFS_close] File handler not valid.\n");

	free(f);
	return 0;
}

//close a dir handler
int SimpleFS_closedir(DirectoryHandle* d){
	//checking
	assert(d != 0 && "[SimpleFS_closedir] Dreictory handler not valid.\n");

	free(d);
	return 0;

}


// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
int SimpleFS_format(SimpleFS* fs, DiskDriver* disk0, char* filename, int num_block){
	//checking
	assert(fs && "[SimpleFS_format] File system not valid");
	assert(disk0 && "[SimpleFS_format] Disk not valid");

	//restore the disk
	DiskDriver_init(disk0, "disk0.dat", 1024);
	DiskDriver_close(disk0, 1);

	//Reopen it
	DiskDriver_open(disk0, "disk0.dat", 1024);

	//Reinit the fs
	SimpleFS_init(fs, disk0);

	return 0;
}
