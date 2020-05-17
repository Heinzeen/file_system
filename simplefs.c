#include "simplefs.h"
#include <stdio.h>


/*
typedef struct {
	BlockHeader header;
	FileControlBlock fcb;
	int num_entries;
	int file_blocks[ (BLOCK_SIZE
			-sizeof(BlockHeader)
			-sizeof(FileControlBlock)
			-sizeof(int))/sizeof(int) ];
} FirstDirectoryBlock;

// this is in the first block of a chain, after the header
typedef struct {
	int directory_block; // first block of the parent directory
	int block_in_disk;	 // repeated position of the block on the disk
	char name[128];
	int size_in_bytes;
	int size_in_blocks;
	int is_dir;					// 0 for file, 1 for dir
} FileControlBlock;

// header, occupies the first portion of each block in the disk
// represents a chained list of blocks
typedef struct {
	int previous_block; // chained list (previous block)
	int next_block;		 // chained list (next_block)
	int block_in_file; // position in the file, if 0 we have a file control block
} BlockHeader;
*/

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
	FileControlBlock fcb = {.directory_block = 0, .block_in_disk = 0, .name = "/", .size_in_bytes = 0, .size_in_blocks = 1, .is_dir = 1};

	FirstDirectoryBlock fdb = { .header = bh, .fcb = fcb, .num_entries = 1};		//TODO add the file_blocks
	
	//write it into the block 0
	int res = DiskDriver_writeBlock(disk, &fdb, 0, sizeof(FirstDirectoryBlock), 0);
	check_errors(res , -1, "[SimpleFS_init] Cannot write the block.");

	return 0;
}

//TODO check for integrity
int SimpleFS_check(SimpleFS* fs){


	return 0;

}

// open an already made filse system
// returns 0 if everything is fine, and if the fs is good
int SimpleFS_open(SimpleFS* fs, DiskDriver* disk){
	//checking
	assert(fs && "[SimpleFS_init] Fs not valid.");
	assert(disk && "[SimpleFS_init] Disk not valid.");

	fs->disk = disk;

	if(SimpleFS_check(fs))
		return -1;

	fs->root_block = 0;
	fs->current_dir_block = 0;


	return 0;
}

//TODO correct this

// creates the inital structures, the top level directory
// has name "/" and its control block is in the first position
// it also clears the bitmap of occupied blocks on the disk
// the current_directory_block is cached in the SimpleFS struct
// and set to the top level directory
int SimpleFS_format(SimpleFS* fs){
	//checking
	assert(fs && "[SimpleFS_format] Fs not valid.");
	assert(fs->disk && "[SimpleFS_format] Disk not valid.");
	assert(fs->disk->name && "[SimpleFS_format] Disk name not valid.");

	int res;
	//Reinitialize everything at 0
	DiskDriver_init(fs->disk, fs->disk->name, fs->disk->header->num_blocks);
	DiskDriver_close(fs->disk, 1);

	//reopen it
	DiskDriver_open(fs->disk, fs->disk->name, fs->disk->header->num_blocks);
	res = SimpleFS_init(fs, fs->disk);
	if(res)
		return -1;
	return 0;


}
