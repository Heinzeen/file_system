#pragma once
#include "bitmap.h"
#include "auxiliary.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>


#define BLOCK_SIZE 512
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
	char* name;
} DiskDriver;

/**
	 The blocks indices seen by the read/write functions 
	 have to be calculated after the space occupied by the bitmap
*/

// opens the file (creating it if necessary)
// allocates the necessary space on the disk
// calculates how big the bitmap should be
// if the file was new
// compiles a disk header, and fills in the bitmap of appropriate size
// with all 0 (to denote the free space);
void DiskDriver_init(DiskDriver* disk, const char* filename, int num_blocks);

//close the disk driver, free the elements and close the descriptor
void DiskDriver_close(DiskDriver* disk, int n);

//open the disk driver, load everything from the file
void DiskDriver_open(DiskDriver* disk, const char* filename, int num_blocks);

// reads the block in position block_num
// returns -1 if the block is free accrding to the bitmap
// 0 otherwise
char* DiskDriver_readBlock(DiskDriver* disk, int block_num, int block_offset);

// writes a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num, int count, int block_offset);

// frees a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num);

// returns the first free blockin the disk from position (checking the bitmap)
int DiskDriver_getFreeBlock(DiskDriver* disk, int start);

// unmap a block
void DiskDriver_unmapBlock(void* ptr);

// writes the data (flushing the mmaps)
int DiskDriver_flush(DiskDriver* disk);
