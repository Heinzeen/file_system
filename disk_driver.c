#include "disk_driver.h"
#include "simplefs.h"



//init the data when we open a new disk
void Init_data(DiskDriver* disk){
	
	//checking
	assert(disk && "[Init_data] Disk pointer not valid.");

	//for both the writes we check errno

	//write the bitmap	
	int res;
	int missing = disk->bitmap->num_bits;
	do{
		res=write(disk->fd, disk->bitmap->entries, missing);
		if(res == -1 && errno != EINTR){
			printf("Error while writing new disk, aborting.\n");
			exit(1);
		}
		missing -= res;
	}while(missing>0);

	char temp[disk->bitmap->num_bits * BLOCK_SIZE];
	memset(temp, 0, disk->bitmap->num_bits * BLOCK_SIZE);
	//write the blocks (all 0 for now)
	missing = disk->bitmap->num_bits * BLOCK_SIZE;
	do{
		res=write(disk->fd, temp, missing);
		if(res == -1 && errno != EINTR){
			printf("Error while writing new disk, aborting.\n");
			exit(1);
		}
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

	dh->num_blocks = num_blocks;
	dh->bitmap_entries = num_blocks;
	dh->free_blocks = num_blocks; 					//in the start, every block is a free block
	dh->first_free_block = 0;

	//initialize the BlockHeader
	LoadedBlockHead* bh = malloc(sizeof(LoadedBlockHead));
	bh->first = 0;
	bh->last = 0;
	bh->size = 0;
	
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
	disk->fd = disk_desc;
	disk->name = filename;
	disk->blockhead = bh;


	//init the disk, all 0 at the moment
	Init_data(disk);

}


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

	dh->num_blocks = num_blocks;
	dh->bitmap_entries = num_blocks;
	dh->free_blocks = BitMap_analyze(bitmap, num_blocks);		//in the start, every block is a free block
	dh->first_free_block = 0;

	//initialize the BlockHeader
	LoadedBlockHead* bh = malloc(sizeof(LoadedBlockHead));
	bh->first = 0;
	bh->last = 0;
	bh->size = 0;


	//set the values
	disk->header = dh;
	disk->bitmap = bitmap;
	disk->fd = disk_desc;
	disk->name = filename;
	disk->blockhead = bh;


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

	DiskDriver_flush(disk);
	
	//freeing the memory
	assert(disk->header && "[DiskDriver_close] Disk header value is not valid.");
	free(disk->header);
	assert(disk->bitmap && "[DiskDriver_close] Disk bitmap value is not valid.");
	free(disk->bitmap);
	assert(disk->blockhead && "[DiskDriver_close] Disk blockhead value is not valid.");
	free(disk->blockhead);

}


// reads the block in position block_num (map it into the process)
char* DiskDriver_readBlock(DiskDriver* disk, int block_num, int block_offset){
	
	//checking
	assert(disk && "[DiskDriver_readBlock] Disk pointer not valid.");
	assert(block_num>=0 && "[DiskDriver_readBlock] Invalid block num");
	assert(block_offset <= BLOCK_SIZE && block_offset >= 0 && "[DiskDriver_readBlock] Invalid count or block_offset.");

	char *dest = getBlockAddress(disk, block_num);

	return dest + block_offset;

}

// writes a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_writeBlock(DiskDriver* disk, void* src, int block_num, int count, int block_offset){

	//checking
	assert(disk && "[DiskDriver_writeBlock] Disk pointer not valid.");
	assert(block_num>=0 && "[DiskDriver_writeBlock] Invalid block num");
	assert(count + block_offset <= BLOCK_SIZE && count > 0 && block_offset >= 0 && "[DiskDriver_writeBlock] Invalid count or block_offset.");



	//get the block address
	char *dest = getBlockAddress(disk, block_num);

	memcpy(dest + block_offset, src, count);

	disk->bitmap->entries[block_num] = 1;

	return 0;
}

// unmap a block
void DiskDriver_unmapBlock(void* ptr){
	//checking
	assert(ptr && "[DiskDriver_unmapBlock] Block's pointer not valid.");
	munmap(ptr, sysconf(_SC_PAGE_SIZE));
}

// writes the data (flushing the mmaps)
void DiskDriver_flush(DiskDriver* disk){
	//checking
	assert(disk && "[DiskDriver_flush] Disk pointer not valid.");
	LoadedBlockHead* bh = disk->blockhead;
	LoadedBlock* temp = bh->first;
	while(temp){
		DiskDriver_unmapBlock(temp->memory);
		LoadedBlock* temp1 = temp;
		temp = temp->next;
		free(temp1);
	}
}

// frees a block in position block_num, and alters the bitmap accordingly
// returns -1 if operation not possible
int DiskDriver_freeBlock(DiskDriver* disk, int block_num){
	//checking
	assert(disk && "[DiskDriver_freeBlock] Disk pointer not valid.");
	if(block_num<=0)
		return -1;


	//change the bitmap and add 1 to the number of free items
	if(disk->bitmap->entries[block_num] != 0){	//just in case we are making a double free
		disk->bitmap->entries[block_num] = 0;
		disk->header->free_blocks += 1;
	}

	return 0;
}

// returns the first free blockin the disk from position (checking the bitmap)
// or -1 if there is no block from that position
int DiskDriver_getFreeBlock(DiskDriver* disk, int start){
	//checking
	assert(disk && "[DiskDriver_getFreeBlock] Disk pointer not valid.");

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



void* getBlockAddress(DiskDriver* disk, int block_num){
	assert(disk && "[getBlockAddress] Disk not valid");
	//we need to do some math because we can only set an offsett which is a multiple of sysconf(_SC_PAGE_SIZE)
	int page_len = sysconf(_SC_PAGE_SIZE);
	
	//printf("Looking for %d\n", block_num);

	//check if we already mapped that portion of memory
	LoadedBlock* temp = disk->blockhead->first;
	while(temp){
		if(block_num >= temp->block_start && block_num <= temp->block_end){
			//printf("Found in [%d, %d]\n", temp->block_start, temp->block_end);
			if(temp->block_start == 0)
				return temp->memory + (block_num - temp->block_start) * BLOCK_SIZE + disk->header->num_blocks;

			else				//used because we have an offset (bitmap) in the first block
				return temp->memory + (block_num - temp->block_start) * BLOCK_SIZE;

		}
		temp = temp->next;
	}


	//since it was not already mapped, map it

	LoadedBlock* lb = malloc(sizeof(LoadedBlock));
	lb->prev = 0;
	lb->next = 0;
	lb->block_start = (block_num + (disk->header->num_blocks / BLOCK_SIZE)) / (page_len / BLOCK_SIZE) * (page_len / BLOCK_SIZE);
	if(lb->block_start != 0)
		lb->block_start -= (disk->header->num_blocks / BLOCK_SIZE);			//Bitmap < 4096, bitmap % BLOCK_SIZE == 0
	lb->block_end = (block_num + (disk->header->num_blocks / BLOCK_SIZE)) / (page_len / BLOCK_SIZE) * (page_len/ BLOCK_SIZE) + (page_len / BLOCK_SIZE) - 1 - (disk->header->num_blocks / BLOCK_SIZE);

	int offset;
	if(block_num < page_len/BLOCK_SIZE - (disk->header->num_blocks / BLOCK_SIZE))
		offset = disk->header->num_blocks + BLOCK_SIZE*block_num;
	else
		offset = BLOCK_SIZE*(block_num- lb->block_start);

	
	char* dest = mmap(0, page_len, PROT_READ | PROT_WRITE, MAP_SHARED, disk->fd, ((block_num * BLOCK_SIZE + disk->header->num_blocks) / page_len) * page_len);
	check_errors((long int)dest, -1, "[getBlockAddress] Cannot map the block.");

	lb->memory = dest;


	offset %= page_len;

	//and add it to our list

	//if it is the first one
	temp = disk->blockhead->first;
	if(temp == 0){
		//printf("Added first[%d, %d]\n", lb->block_start, lb->block_end);
		disk->blockhead->first = lb;
		disk->blockhead->last = lb;
		return dest + offset;
	}

	//if we have to add it in the middle
	while(temp){
		if(lb->block_end < temp->block_start){
			lb->next = temp;
			lb->prev = temp->prev;
			if(temp->prev == 0)
				disk->blockhead->first = lb;
			else
				temp->prev->next = lb;
			//printf("Added middle[%d, %d]\n", lb->block_start, lb->block_end);
			temp->prev = lb;
			return dest + offset;
		}
		temp = temp->next;		
	}
	//if we are here, we reached the end of the list
	temp = disk->blockhead->last;
	temp->next = lb;
	lb->prev = temp;
	lb->next = 0;
	disk->blockhead->last = lb;
	//printf("Added last[%d, %d]\n", lb->block_start, lb->block_end);
	return dest + offset;


}
