#include "auxiliary.h"

void debug_print(char* msg){	//TODO we can add some condition
	printf("%s\n", msg);
}


void check_errors(int result, int not_wanted, char* msg){
	if(result == not_wanted){
		printf("%s\n", msg);
		exit(1);
	}
}
