#include "bitmap.h"

/*
typedef struct{
  int num_bits;
  char* entries;
}  BitMap;

typedef struct {
  int entry_num;
  char bit_num;
} BitMapEntryKey;
*/


// initialize a bitmap
void BitMap_init(BitMap* bmap){
	//checking
	assert(bmap && "[BitMap_init] Bmap pointer not valid.");

	debug_print("Initializing the bitmap");

	//create the entries and set them to 0
	bmap->entries = malloc(bmap->num_bits);
	assert(bmap->entries && "[BitMap_init] Cannot allocate the bitmap's entries.");

	memset(bmap->entries, 0, bmap->num_bits);
	assert(bmap->entries && "[BitMap_init] Cannot set the bitmap's entries to 0.");
}

void BitMap_free(BitMap* bmap){
	//checking
	assert(bmap && "[BitMap_close] Bmap pointer not valid.");


	//freeing the memory
	assert(bmap->entries && "[DiskDriver_close] Disk header value is not valid.");
	free(bmap->entries);

}

//check how many blocks are in use
int BitMap_analyze(BitMap* bmap, int num_blocks){
	//checking
	assert(bmap && "[BitMap_analyze] Bmap pointer not valid.");
	assert(num_blocks > 0 && "[BitMap_analyze] Bmap pointer not valid.");

	//iterate through the bitmap and check for the values
	int i;
	for(i=0; i<num_blocks; i++){
		assert((bmap->entries[i] == 0 || bmap->entries[i] == 1) && "[BitMap_analyze] Bad bitmap.");
		num_blocks -= bmap->entries[i];
	}
	return num_blocks;

}


void BitMap_unmap(BitMap* bmap){
	//checking
	assert(bmap && "[BitMap_close] Bmap pointer not valid.");


	//freeing the memory
	assert(bmap->entries && "[DiskDriver_close] Disk header value is not valid.");
	munmap(bmap->entries, bmap->num_bits);

}
