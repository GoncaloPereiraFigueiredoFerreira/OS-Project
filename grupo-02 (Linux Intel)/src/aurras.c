#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#define MAX (PIPE_BUF-sizeof(pid_t)-1) 
// codigo cliente
void *packet;
int fOut;
int fIn;

void handler_term(int n){
	void *packet2;
	if((packet2 = malloc(PIPE_BUF))==(void*)-1)perror("Malloc");
	read(fIn,packet2,PIPE_BUF);
	printf("%s\n", (char*)packet2 );
	printf("Finished!\n");
	free(packet2);
	free(packet);
	if(close(fIn)==-1)perror("Close");
	if(close(fOut)==-1)perror("Close");
	_exit(0);
}

void handler_sigusr1(int n){
	printf("Erro!Demasiados filtros.\n");
	if(close(fIn)==-1)perror("Close");
	if(close(fOut)==-1)perror("Close");
	free(packet);
	_exit(0);
}

void handler_sigusr2(int n){
	printf("Processing!\n");
}

int main (int argc, char * argv[]){

	int n;
	if (argc>=1 && ((strcmp(argv[1],"transform") == 0) || (argc == 2 && strcmp(argv[1],"status") == 0))){


		fOut = open("tmp/pIn",O_WRONLY);
		fIn = open("tmp/pOut",O_RDONLY | O_NONBLOCK);
		if(fOut == -1 || fIn == -1) {printf("Server close");return 0;}
		if((packet = malloc(PIPE_BUF))==(void*)-1)perror("Malloc");
		char *buffer = packet + sizeof(pid_t);
		pid_t *pid = packet;
		*pid = getpid();
		buffer[MAX+1] = '\n';
		if(signal(SIGTERM,handler_term)==(void*)-1)perror("SigTerm");
		if(signal(SIGUSR1,handler_sigusr1)==(void*) -1)perror("SigUsr1");
		if(signal(SIGUSR2,handler_sigusr2)==(void*) -1)perror("SigUsr2");

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
			buffer[0] = (i == 1)?'\r':'\n';
			buffer[n] = (i == (argc-1))? '\0' : '\n';
			if(write(fOut,packet,n+sizeof(pid_t)+1)==-1)perror("Write");
		}

		if(argc > 1){
			printf("Pending!\n");
			while(1)pause();
		}
		// abre os pipes
		// envia informaçao e o seu pid
		// espera por sinal
		// prints da informaçao recebida
		free(packet);
	}
	
	else {
		printf("Erro: Comando não reconhecido \n");
		printf("Por favor use:\n");
		printf("aurras status -> Receber estado do processamento do servidor\n");
		printf("aurras transform <inputFile> <outputFile> Filter1 Filter2 ... -> aplicação de filtros no inputFile\n");
	}
}
