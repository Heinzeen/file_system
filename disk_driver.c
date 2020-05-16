#include "disk_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

/*
// this is stored in the 1st block of the disk
typedef struct {
	int num_blocks;
	int bitmap_blocks;	 // how many blocks in the bitmap
	int bitmap_entries;	// how many bytes are needed to store the bitmap
	
	int free_blocks;		 // free blocks
	int first_free_block;// first block index
} DiskHeader; 

typedef struct {
	DiskHeader* header; // mmapped
	BitMap* bitmap;	// mmapped (bitmap)
	int fd; // for us
} DiskDriver;

*/


//init the data when we open a new disk
void Init_data(DiskDriver* disk){
	
	//checking
	assert(disk && "[Init_data] Disk pointer not valid.");
	

	//write the bitmap	
	int res;
	int missing = disk->bitmap->num_bits;
	do{			//don't think about int at the moment TODO
		res=write(disk->fd, disk->bitmap->entries, missing);
		check_errors(res, -1, "[Init_data] Cannot write on disk.");
		missing -= res;
	}while(missing>0);

	//TODO maybe change this?
	char temp[disk->bitmap->num_bits * BLOCK_SIZE];
	memset(temp, 0, disk->bitmap->num_bits * BLOCK_SIZE);
	//write the blocks (all 0 for now)
	missing = disk->bitmap->num_bits * BLOCK_SIZE;
	do{			//don't think about int at the moment TODO
		res=write(disk->fd, temp, missing);
		check_errors(res, -1, "[Init_data] Cannot write on disk.");
		missing -= res;

	}while(missing>0);

}



// opens the file (creating it if necessary)
// allocates the necessary space on the disk
// calculates how big the bitmap should be
// if the file was new
// compiles a disk header, and fills in the bitmap of appropriate size
// with all 0 (to denote the free space);
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks){
	//paranoid mode
	assert(disk && "[DiskDriver_init] Disk pointer not valid.");
	assert(filename && "[DiskDriver_init] Disk's filename not valid.");


	debug_print("Initializing the disk driver.");


	//initialize the header with its fields
	DiskHeader* dh = malloc(sizeof(DiskHeader));
	assert(dh && "[DiskDriver_init] Cannot allocate disk header.");

	dh->num_blocks = num_blocks;					//TODO do we need the bitmap_blocks field?
	dh->bitmap_entries = num_blocks * sizeof(BitMapEntryKey);
	dh->free_blocks = num_blocks; 					//in the start, every block is a free block
	dh->first_free_block = 0;
	
	
	//initialize the bitmap
	BitMap* bitmap = malloc(sizeof(BitMap));	//size of the bitmap
	assert(bitmap && "[DiskDriver_init] Cannot allocate bitmap.");
	bitmap->num_bits = num_blocks;
	BitMap_init(bitmap);


	//opening the file
	int disk_desc = open(filename, O_RDWR | O_CREAT, 0644);
	check_errors(disk_desc, 0, "[DiskDriver_init] Cannot open descriptor.");



	//set the values
	disk->header = dh;
	disk->bitmap = bitmap;
	disk->fd=disk_desc;


	//init the disk, all 0 at the moment
	Init_data(disk);

}


//close the disk driver, free the elements and close the descriptor
void DiskDriver_close(DiskDriver* disk){

	//checking
	assert(disk && "[DiskDriver_close] Disk pointer not valid.");

	debug_print("Closing the disk driver.");

	BitMap_close(disk->bitmap);

	//closing the file
	int ret = close(disk->fd);
	check_errors(ret, -1, "[DiskDriver_close] Cannot close the descriptor.");
	
	//freeing the memory
	assert(disk->header && "[DiskDriver_close] Disk header value is not valid.");
	free(disk->header);
	assert(disk->bitmap && "[DiskDriver_close] Disk header value is not valid.");
	free(disk->bitmap);

}



// reads the block in position block_num
// returns -1 if the block is free accrding to the bitmap
// 0 otherwise
int DiskDriver_readBlock(DiskDriver* disk, void* dest, int block_num){
	
	//checking
	assert(disk && "[DiskDriver_readBlock] Disk pointer not valid.");
	assert(block_num>=0 && "[DiskDriver_readBlock] Invalid block num");
	
	return 0;	//placeholder




}
