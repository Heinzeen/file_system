#include <stdio.h>
#include "simplefs.h"
#include "bins.h"

//global variables, for ease of use
DirectoryHandle* rh;		//root handler
DirectoryHandle* current;	//current handler
SimpleFS* sfs;			//fs
DiskDriver* disk;		//disk


//load a predefined fs
void load_sfs(void){

	disk = malloc(sizeof(DiskDriver));
	sfs = malloc(sizeof(SimpleFS));
	DiskDriver_open(disk, "disk0.dat", 1024);
	rh = SimpleFS_open(sfs, disk);

	//adding stuff to make the shell work
	current = rh;

}

//close the fs
void close_sfs(void){

	//shell's stuff
	if(current != rh)
		SimpleFS_closedir(current);

	//fs stuff
	SimpleFS_closedir(rh);
	free(sfs);
	DiskDriver_close(disk, 0);
	free(disk);

}


void format(void){

	//close
	close_sfs();

	//format
	disk = malloc(sizeof(DiskDriver));
	sfs = malloc(sizeof(SimpleFS));
	SimpleFS_format(sfs, disk, "disk0.dat", 1024);

	rh = SimpleFS_open(sfs, disk);

	//adding stuff to make the shell work
	current = rh;

}

void info(char* name){

	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;

	//check if we want root
	if(!strncmp(name, "/", 1)){
		SimpleFS_printDirData(rh);
		return;
	}

	//take the handler
	FileHandle* temp = SimpleFS_openFile(current, name);
	if(temp == 0){
		printf("Cannot find the item.\n");
		return;
	}
	if(temp->fcb->fcb.is_dir){	//we're in a directory
		SimpleFS_printDirData((DirectoryHandle*) temp);
		SimpleFS_closedir((DirectoryHandle*) temp);
	}
	else{
		SimpleFS_printFileData(temp);
		SimpleFS_close(temp);
	}
}

void print_help(void){
	printf("Command list:\n");
	printf("cd\t\tchange directory; use \"cd /\"to do in root.\n");
	printf("exit\t\texit the shell\n");
	printf("format\t\tformat the current fs.\n");
	printf("info\t\tprint the info about a file.\n");
	printf("ls\t\tshow the content of the actual dir.\n");
	printf("mkdir\t\tmake a dir.\n");
	printf("read filename\tread filename; you will be asked for the dimension.\n");
	printf("rm\t\tremove a file or a dir.\n");
	printf("touch\t\tcreate a file.\n");
	printf("write filename\twrite filename; you will be asked for the dimension.\n");
	printf("writerand filename\twrite \"random\" bytes in a file; you will be asked for the dimension.\n");
}

void cd(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;

	if(!strcmp(name, "/")){
		if(current != rh)
			SimpleFS_closedir(current);
		current = rh;
		return;
	}


	DirectoryHandle* new = SimpleFS_openDir(current, name);

	if(current != rh)
		SimpleFS_closedir(current);

	if(new)
		current = new;
	else
		printf("Cannot open dir %s.\n", name);
}

void mkdir(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;
	//call the function
	SimpleFS_mkDir(current, name);
}

void rm(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;


	int res = SimpleFS_remove(current, name);

	if(res)
		printf("Cannot remove the file or the directory.\n");

}


void touch(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;

	//create the file
	SimpleFS_createFile(current, name);
}

void read_file(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;

	//check if the file exists
	if(!SimpleFS_checkname(current, name)){
		printf("File doesn't exist!\n");
		return;
	}

	//ask for the parameters
	int n;
	char c;
	printf("Number of bytes: ");
	scanf("%d", &n);
	scanf("%c", &c);		//the first scanf doesn't read the \n, so the fgets will read it and exit unless i do this

	//check
	if(n<0){
		printf("Please, insert a positive n.\n");
		return;
	}



	//open the file
	FileHandle* fh = SimpleFS_openFile(current, name);

	if(n==0)
		n = fh->fcb->fcb.size_in_bytes;

	char data[n+1];

	SimpleFS_read(fh, data, n);

	printf("%s\n", data);

	//close it
	SimpleFS_close(fh);

}

void write_file(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;

	//check if the file exists
	if(!SimpleFS_checkname(current, name)){
		printf("File doesn't exist!\n");
		return;
	}

	//ask for the parameters
	int n;
	char c;
	printf("Number of bytes: ");
	scanf("%d", &n);
	scanf("%c", &c);		//the first scanf doesn't read the \n, so the fgets will read it and exit unless i do this

	//check
	if(n<=0){
		printf("Please, insert a positive, not-null number.\n");
		return;
	}

	char data[n+1];
	memset(data, 0, n);		//set everything to zero

	printf("Insert the data: ");
	fgets(data, n+1, stdin);	//while inserting, try not to let the string terminator in

	//open the file
	FileHandle* fh = SimpleFS_openFile(current, name);

	SimpleFS_write(fh, data, n);

	//close it
	SimpleFS_close(fh);
}

//generate n "random" bytes
void rand_gen(char* data, int n){
	int i;
	for(i=0;i<n;i++){
		data[i]=32+rand()%96;		//generate them from 32 to 128 (i don't want random null bytes or other strange things inside the files)
	}
}
void write_file_rand(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;


	//check if the file exists
	if(!SimpleFS_checkname(current, name)){
		printf("File doesn't exist!\n");
		return;
	}

	//ask for the parameters
	int n;
	char c;
	printf("Number of bytes: ");
	scanf("%d", &n);
	scanf("%c", &c);		//the first scanf doesn't read the \n, so the fgets will read it and exit unless i do this

	//check
	if(n<=0){
		printf("Please, insert a positive, not-null number.\n");
		return;
	}

	char data[n+1];
	memset(data, 0, n);		//set everything to zero

	//generate
	rand_gen(data, n);
	//open the file
	FileHandle* fh = SimpleFS_openFile(current, name);

	SimpleFS_write(fh, data, n);

	//close it
	SimpleFS_close(fh);
}

int main(int agc, char** argv) {
	//Be always kind with those who have to examine you <3
	printf("Welcome to SimpleFS! OS's project by Matteo Marini.\n");
	printf("Type \"help\" for help.\n");

	//load the fs
	load_sfs();


	char msg[64];
	while(1){
		printf("SimpleFS:%s $ ", current->dcb->fcb.name);
		fgets(msg, 64, stdin);

		//help
		if(!strncmp(msg, "cd", 2))
			cd(msg+3);

		//exit
		else if(!strncmp(msg, "exit", 4)){
			printf("Bye!\n");
			break;
		}

		else if(!strncmp(msg, "format", 6))
			format();

		//help
		else if(!strncmp(msg, "help", 4))
			print_help();

		//hello
		else if(!strncmp(msg, "hello", 4))
			printf("Hello :)\n");

		//info
		else if(!strncmp(msg, "info", 4))
			info(msg+5);

		else if(!strncmp(msg, "ls", 2))
			my_ls(current);

		else if(!strncmp(msg, "mkdir", 5))
			mkdir(msg+6);

		else if(!strncmp(msg, "touch", 5))
			touch(msg+6);

		else if(!strncmp(msg, "read", 4))
			read_file(msg+5);

		else if(!strncmp(msg, "rm", 2))
			rm(msg+3);

		else if(!strncmp(msg, "writerand", 9))
			write_file_rand(msg+10);

		else if(!strncmp(msg, "write", 5))
			write_file(msg+6);


		//you need help?
		else
			printf("Command unknown, type \"help\" for help.\n");
	}

	//close the fs
	close_sfs();
	return 0;

}
