CCOPTS= -Wall -g -std=gnu99 -Wstrict-prototypes
LIBS= 
CC=gcc
AR=ar


BINS= fs_shell

OBJS = simplefs.o\
	bitmap.o\
	disk_driver.o\
	auxiliary.o\
	bins.c

HEADERS=bitmap.h\
	disk_driver.h\
	simplefs.h\
	linked_list.h\
	bins.h

%.o:	%.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@  $<

.phony: clean all


all:	$(BINS) 

fs_shell: fs_shell.c $(OBJS) 
	$(CC) $(CCOPTS)  -o $@ $^ $(LIBS)

clean:
	rm -rf *.o *~  $(BINS)
