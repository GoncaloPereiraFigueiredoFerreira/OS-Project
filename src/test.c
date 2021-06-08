#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>

#include <stdio.h>

int main(){
	if(!fork()){
		execlp("wc",NULL,NULL);
	}
}