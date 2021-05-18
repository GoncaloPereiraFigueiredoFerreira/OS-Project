#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>

#include <stdio.h>

#define MAX (PIPE_BUF-sizeof(pid_t)-1) 
// codigo cliente

int main (int argc, char * argv[]){

	int n;
	int fIn = open("pOut1",O_RDONLY);
	int fOut = open("pIn",O_WRONLY);
	void *packet = malloc(PIPE_BUF);
	char *buffer = packet + sizeof(pid_t);
	pid_t *pid = packet;
	*pid = getpid();
	buffer[MAX+1] = '\n';

	for(int i = 1; i < argc;i++){
		n = 1;
		for(int j = 0; argv[i][j] != '\0';j++){
			if(n == MAX){
				buffer[0]  = '\t';
				write(fOut,packet,PIPE_BUF);
				n = 1;
			}
			buffer[n] = argv[i][j];
			n++;
		}
		buffer[0] = '\n';
		buffer[n] = (i == (argc-1))? '\0' : '\n';
		write(fOut,packet,n+sizeof(pid_t)+1);
	}

	// abre os pipes
	// envia informaçao e o seu pid
	// espera por sinal
	// prints da informaçao recebida
	//close(fIn);
	close(fOut);
	free(packet);
}