#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> //memset
#include "auxiliary.h"
#include <sys/mman.h>


typedef struct{
  int num_bits;
  char* entries;
}  BitMap;

typedef struct {
  int entry_num;
  char bit_num;
} BitMapEntryKey;

// initialize a bitmap
void BitMap_init(BitMap* bmap);

// close a bitmap that dinamically allocated
void BitMap_free(BitMap* bmap);

// close a bitmap that was mapped
void BitMap_unmap(BitMap* bmap);

//check how many blocks are in use
int BitMap_analyze(BitMap* bmap, int num_blocks);

// converts a block index to an index in the array,
// and a char that indicates the offset of the bit inside the array
BitMapEntryKey BitMap_blockToIndex(int num);

// converts a bit to a linear index
int BitMap_indexToBlock(int entry, uint8_t bit_num);

// returns the index of the first bit having status "status"
// in the bitmap bmap, and starts looking from position start
int BitMap_get(BitMap* bmap, int start, int status);

// sets the bit at index pos in bmap to status
int BitMap_set(BitMap* bmap, int pos, int status);
