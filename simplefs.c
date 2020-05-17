#include "simplefs.h"
#include <stdio.h>

/*
// this is a file handle, used to refer to open files
typedef struct {
	SimpleFS* sfs;			// pointer to memory file system structure
	FirstFileBlock* fcb;		// pointer to the first block of the file(read it)
	FirstDirectoryBlock* directory;	// pointer to the directory where the file is stored
	BlockHeader* current_block;	// current block in the file
	int pos_in_file;		// position of the cursor
} FileHandle;

typedef struct {
	SimpleFS* sfs;			// pointer to memory file system structure
	FirstDirectoryBlock* dcb;	// pointer to the first block of the directory(read it)
	FirstDirectoryBlock* directory;	// pointer to the parent directory (null if top level)
	BlockHeader* current_block;	// current block in the directory
	int pos_in_dir;			// absolute position of the cursor in the directory
	int pos_in_block;		// relative position of the cursor in the block
} DirectoryHandle;

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

//TODO check for integrity (disk)
int SimpleFS_check(SimpleFS* fs){


	return 0;

}

/*
typedef struct {
	SimpleFS* sfs;			// pointer to memory file system structure
	FirstDirectoryBlock* dcb;	// pointer to the first block of the directory(read it)
	FirstDirectoryBlock* directory;	// pointer to the parent directory (null if top level)
	BlockHeader* current_block;	// current block in the directory
	int pos_in_dir;			// absolute position of the cursor in the directory
	int pos_in_block;		// relative position of the cursor in the block
} DirectoryHandle;
*/

// open an already made filse system
// returns 0 if everything is fine, and if the fs is good
DirectoryHandle* SimpleFS_open(SimpleFS* fs, DiskDriver* disk){
	//checking
	assert(fs && "[SimpleFS_init] Fs not valid.");
	assert(disk && "[SimpleFS_init] Disk not valid.");

	fs->disk = disk;

	if(SimpleFS_check(fs))
		return 0;

	//root's handler
	/*DirectoryHandle* rh = malloc(sizeof(DirectoryHandle));
	rh->sfs = fs;*/

	return 0;
}

