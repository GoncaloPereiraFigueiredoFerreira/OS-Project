#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>


typedef struct filters{
	char *name;
	char *cmd;
	int max;
	int curr;
}*FLTRS;

char* read_line(int fd,int* bytes_read){
    *bytes_read=0;
    int total_bytes_read=0;
    int max_size = 1024;
    char* res = malloc(sizeof(char)*max_size);
    int n_bytes_read=0;
    
    while((n_bytes_read=read(fd,res+total_bytes_read,1))>0){
        total_bytes_read++;
        if(res[total_bytes_read-1]=='\n'){
            res[total_bytes_read-1] = '\0';
            break;
        }
        if(total_bytes_read==max_size){
            max_size*=2; 
            res = realloc(res,sizeof(char) * (max_size));
        }
    }
    *bytes_read = total_bytes_read-1;
    return res;
}