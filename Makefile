CCOPTS= -Wall -g -std=gnu99 -Wstrict-prototypes
LIBS= 
CC=gcc
AR=ar


BINS= simplefs_test

OBJS = simplefs.o\
	bitmap.o\
	disk_driver.o\
	auxiliary.o

HEADERS=bitmap.h\
	disk_driver.h\
	simplefs.h\
	linked_list.h

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

.phony: clean all


all:	$(BINS) 

so_game: simplefs_test.c $(OBJS) 
	$(CC) $(CCOPTS)  -o $@ $^ $(LIBS)

clean:
	rm -rf *.o *~  $(BINS)
