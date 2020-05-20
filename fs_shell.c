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
	printf("rm\t\tremove a file or a dir.\n");
	printf("touch\t\tcreate a file.\n");
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

	if(current != rh)
		SimpleFS_closedir(current);
	DirectoryHandle* new = SimpleFS_openDir(current, name);
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
		printf("Cannot remove the file or the directory.");

}


void touch(char* name){
	//take away the \n
	int len = strlen(name);
	name[len-1] = 0;

	//create the file
	SimpleFS_createFile(current, name);
}


int main(int agc, char** argv) {
	//Be always kind with those who have to examine you <3
	printf("Welcome to SimpleFS! OS's project by Matteo Marini.\n");
	printf("Type \"help\" for help.\n");

	//load the fs
	load_sfs();

	char msg[64];
	while(1){
		printf("%s >>", current->dcb->fcb.name);
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

		else if(!strncmp(msg, "rm", 2))
			rm(msg+3);

		//you need help?
		else
			printf("Command unknown, type \"help\" for help.\n");
	}

	//close the fs
	close_sfs();
	return 0;

}
