#include "disk_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

/*
// this is stored in the 1st block of the disk
typedef struct {
	int num_blocks;
	int bitmap_blocks;	// how many blocks in the bitmap
	int bitmap_entries;	// how many bytes are needed to store the bitmap
	
	int free_blocks		// free blocks
	int first_free_block;	// first block index
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
	assert(num_blocks>0 && "[DiskDriver_init] Block num not valid.");


	debug_print("Initializing the disk driver.");


	//initialize the header with its fields
	DiskHeader* dh = malloc(sizeof(DiskHeader));
	assert(dh && "[DiskDriver_init] Cannot allocate disk header.");

	dh->num_blocks = num_blocks;					//TODO do we need the bitmap_blocks field?
	dh->bitmap_entries = num_blocks;
	dh->free_blocks = num_blocks; 					//in the start, every block is a free block
	dh->first_free_block = 0;
	
	
	//initialize the bitmap
	BitMap* bitmap = malloc(sizeof(BitMap));	//size of the bitmap
	assert(bitmap && "[DiskDriver_init] Cannot allocate bitmap.");
	bitmap->num_bits = num_blocks;
	BitMap_init(bitmap);


	//opening the file
	int disk_desc = open(filename, O_RDWR | O_CREAT, 0644);
	check_errors(disk_desc, -1, "[DiskDriver_init] Cannot open descriptor.");



	//set the values
	disk->header = dh;
	disk->bitmap = bitmap;
	disk->fd=disk_desc;


	//init the disk, all 0 at the moment
	Init_data(disk);

}

//TODO check if the disk is "good"

//open the disk driver, load everything from the file
void DiskDriver_open(DiskDriver* disk, const char* filename, int num_blocks){
	//paranoid mode
	assert(disk && "[DiskDriver_open] Disk pointer not valid.");
	assert(filename && "[DiskDriver_open] Disk's filename not valid.");
	assert(num_blocks>0 && "[DiskDriver_open] Block num not valid.");

	debug_print("Opening the disk.");

	//opening the file
	int disk_desc = open(filename, O_RDWR , 0644);
	check_errors(disk_desc, -1, "[DiskDriver_open] Cannot open descriptor.");

	//initialize the bitmap
	BitMap* bitmap = malloc(sizeof(BitMap));	//size of the bitmap
	assert(bitmap && "[DiskDriver_open] Cannot allocate bitmap.");
	bitmap->num_bits = num_blocks;

	//mmap the bitmap
	bitmap->entries = mmap(0, num_blocks, PROT_READ | PROT_WRITE, MAP_SHARED, disk_desc, 0);
	check_errors((long int)bitmap->entries, -1, "[DiskDriver_open] Cannot map bitmap.");

	//initialize the header with its fields
	DiskHeader* dh = malloc(sizeof(DiskHeader));
	assert(dh && "[DiskDriver_init] Cannot allocate disk header.");

	dh->num_blocks = num_blocks;						//TODO do we need the bitmap_blocks field?
	dh->bitmap_entries = num_blocks;
	dh->free_blocks = BitMap_analyze(bitmap, num_blocks);		//in the start, every block is a free block
	dh->first_free_block = 0;


	//set the values
	disk->header = dh;
	disk->bitmap = bitmap;
	disk->fd = disk_desc;


}

//close the disk driver, free the elements and close the descriptor
void DiskDriver_close(DiskDriver* disk, int n){

	//checking
	assert(disk && "[DiskDriver_close] Disk pointer not valid.");

	debug_print("Closing the disk driver.");

	if (n)					//check if the bitmap was allocated or mapped (n stands for new)
		BitMap_free(disk->bitmap);
	else
		BitMap_unmap(disk->bitmap);

	//closing the file
	int ret = close(disk->fd);
	check_errors(ret, -1, "[DiskDriver_close] Cannot close the descriptor.");
	
	//freeing the memory
	assert(disk->header && "[DiskDriver_close] Disk header value is not valid.");
	free(disk->header);
	assert(disk->bitmap && "[DiskDriver_close] Disk bitmap value is not valid.");
	free(disk->bitmap);

}


//TODO check the method I use to take the offset for the mmap, it won't work with some page_len and BLOCK_SIZEs
// reads the block in position block_num (map it into the process)
char* DiskDriver_readBlock(DiskDriver* disk, int block_num){
	
	//checking
	assert(disk && "[DiskDriver_readBlock] Disk pointer not valid.");
	assert(block_num>0 && "[DiskDriver_readBlock] Invalid block num");


	//we need to do some math because we can only set an offsett which is a multiple of sysconf(_SC_PAGE_SIZE)
	int page_len = sysconf(_SC_PAGE_SIZE);
	
	int offset = disk->header->num_blocks + BLOCK_SIZE*block_num;
	char* dest = mmap(0, (offset % page_len) + BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, disk->fd, offset / page_len);
	check_errors((long int)dest, -1, "[DiskDriver_readBlock] Cannot map the block.");

	return dest + offset;

}

//TODO check the method I use to take the offset for the mmap, it won't work with some page_len and BLOCK_SIZEs
// writes a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num){		//TODO is there any case i cannot do that? (for the -1 retval)
	
	//checking
	assert(disk && "[DiskDriver_writeBlock] Disk pointer not valid.");
	assert(block_num>0 && "[DiskDriver_writeBlock] Invalid block num");


	//we need to do some math because we can only set an offsett which is a multiple of sysconf(_SC_PAGE_SIZE)
	int page_len = sysconf(_SC_PAGE_SIZE);
	
	int offset = disk->header->num_blocks + BLOCK_SIZE*block_num;
	char* dest = mmap(0, (offset % page_len) + BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, disk->fd, (offset / page_len) * page_len);
	check_errors((long int)dest, -1, "[DiskDriver_writeBlock] Cannot map the block.");


	memcpy(dest + offset, src, BLOCK_SIZE);

	disk->bitmap->entries[block_num] = 1;

	DiskDriver_unmapBlock(dest);
	return 0;
}

// unmap a block
void DiskDriver_unmapBlock(void* ptr){
	//checking
	assert(ptr && "[DiskDriver_unmapBlock] Block's pointer not valid.");
	munmap(ptr, BLOCK_SIZE);
}


//TODO do I have to do anything else?
// frees a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num){
	//checking
	assert(disk && "[DiskDriver_freeBlock] Disk pointer not valid.");
	if(block_num<=0)
		return -1;


	//change the bitmap and add 1 to the number of free items
	disk->bitmap->entries[block_num] = 0;
	disk->header->free_blocks += 1;
	return 0;
}

// returns the first free blockin the disk from position (checking the bitmap)
// or -1 if there is no block from that position
int DiskDriver_getFreeBlock(DiskDriver* disk, int start){
	//checking
	assert(disk && "[DiskDriver_freeBlock] Disk pointer not valid.");

	//iterate untill we find a block
	int i;
	if(start == 0)
		start = 1;	//we don't want to check root
	for(i=start; i< disk->header->num_blocks; i++){
		if(disk->bitmap->entries[i] == 0)
			break;
	}
	//check if the cycle didn't end because we run out of blocks
	if(i == disk->header->num_blocks)
		return -1;

	//modify the bitmap
	disk->bitmap->entries[i] = 1;
	disk->header->free_blocks -= 1;
	return i;
}
