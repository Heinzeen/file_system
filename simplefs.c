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
	BlockHeader bh = {.previous_block = 0, .next_block = 0, .block_in_file = 0};
	FileControlBlock fcb = {.first_block = 0, .directory_block = 0, .block_in_disk = 0, .name = "/", .size_in_bytes = 0, .size_in_blocks = 1, .is_dir = 1};

	FirstDirectoryBlock fdb = {.header = bh, .fcb = fcb, .num_entries = 0};		//TODO add the file_blocks
	
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

/*
// this is a file handle, used to refer to open files
typedef struct {
	SimpleFS* sfs;			// pointer to memory file system structure
	FirstFileBlock* fcb;		// pointer to the first block of the file(read it)
	FirstDirectoryBlock* directory;	// pointer to the directory where the file is stored
	BlockHeader* current_block;	// current block in the file
	int pos_in_file;		// position of the cursor
} FileHandle;

// this is in the first block of a chain, after the header
typedef struct {
	int first_block;
	int directory_block; // first block of the parent directory
	int block_in_disk;	 // repeated position of the block on the disk
	char name[128];
	int size_in_bytes;
	int size_in_blocks;
	int is_dir;					// 0 for file, 1 for dir
} FileControlBlock;

typedef struct {
	BlockHeader header;
	FileControlBlock fcb;
	char data[BLOCK_SIZE-sizeof(FileControlBlock) - sizeof(BlockHeader)] ;
} FirstFileBlock;

*/

int SimpleFS_checkname(DirectoryHandle* d, const char* filename){
	int n = d->dcb->num_entries, i;
	FirstFileBlock* ffb;
	for(i=1; i<=n; i++){
		ffb = (FirstFileBlock*) DiskDriver_readBlock(d->sfs->disk, d->dcb->file_blocks[i], 0);
		//printf("%s %s\n", ffb->fcb.name, filename);
		if(!strcmp(ffb->fcb.name, filename))
			return 1;
	}

	return 0;
}

// creates an empty file in the directory d
// returns null on error (file existing, no free blocks)
// an empty file consists only of a block of type FirstBlock
FileHandle* SimpleFS_createFile(DirectoryHandle* d, const char* filename){
	//checking
	assert(d && "[SimpleFS_createFile] Dir not valid.");
	assert(filename && "[SimpleFS_createFile] Filename not valid.");

	//ask for a new block
	int block = DiskDriver_getFreeBlock(d->sfs->disk, 0);
	if(block == -1){
		printf("Cannot allocate a new block.\n");
		return 0;
	}

	//check if there is no other files with the same name
	if(SimpleFS_checkname(d, filename)){
		printf("A file with that name already exists!\n");
		return 0;
	}

	//create the block header
	BlockHeader bh = {.previous_block = -1, .next_block = -1, .block_in_file = block};

	//create fcb
	FileControlBlock fcb = {.first_block = block, .directory_block = d->dcb->fcb.first_block, .block_in_disk = 0, .size_in_bytes = 0, .size_in_blocks = 1, .is_dir = 0};
	memcpy(fcb.name, filename, strlen(filename));

	//create file first block
	FirstFileBlock* ffb = malloc(sizeof(FirstFileBlock));
	ffb->header = bh;
	ffb->fcb = fcb;

	//write on disk
	DiskDriver_writeBlock(d->sfs->disk, ffb, block, sizeof(FirstFileBlock), 0);

	//create the file handler
	FileHandle* fh = malloc(sizeof(FileHandle));
	fh->sfs = d->sfs;
	fh->fcb = ffb;
	fh->directory = d->dcb;
	fh->current_block = (BlockHeader*)getBlockAddress(d->sfs->disk, block);
	fh->pos_in_file = 0;
	
	//add it to the directory
	d->dcb->num_entries += 1;
	d->dcb->file_blocks[d->dcb->num_entries] = block;

	//save the new things on the dir's block
	int res = DiskDriver_writeBlock(d->sfs->disk, d->dcb, 0, sizeof(FirstDirectoryBlock), 0);
	check_errors(res , -1, "[SimpleFS_createFile] Cannot write the block.");


	return fh;
}

// closes a file handle (destroyes it)
int SimpleFS_close(FileHandle* f){


	return 0;
}
