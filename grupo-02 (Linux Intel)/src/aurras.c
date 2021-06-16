#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>

#include <stdio.h>

#define MAX (PIPE_BUF-sizeof(pid_t)-1) 
// codigo cliente


void handler_term(int n){
	int fIn = open("pOut",O_RDONLY | O_NONBLOCK);
	void *packet2 = malloc(PIPE_BUF);
	read(fIn,packet2,PIPE_BUF);
	printf("%s\n", (char*)packet2 );
	_exit(0);
}

int main (int argc, char * argv[]){

	char msg1[] = "pending\n", msg2[] = "done!!\n", msg3[] = "refused\n";
	int n;
	if (arc>=1 && ((strcmp(argv[1],"transform") == 0) || (argc == 2 && strcmp(argv[1],"transform") == 0))){


		int fOut = open("pIn",O_WRONLY);
		int fIn = open("pOut",O_RDONLY);
		void *packet = malloc(PIPE_BUF);
		void *packet2 = malloc(PIPE_BUF);
		char *buffer = packet + sizeof(pid_t);
		pid_t *pid = packet;
		*pid = getpid();
		buffer[MAX+1] = '\n';
		signal(SIGTERM,handler_term);

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

		if(argc > 1){
			write(1,msg1,9);
			pause();
		}
		// abre os pipes
		// envia informaçao e o seu pid
		// espera por sinal
		// prints da informaçao recebida
		close(fIn);
		close(fOut);
		free(packet);
	}
	else {
		write(1,"Erro: Comando não reconhecido \n",strlen("Erro: Comando não reconhecido \n"));7
		write(1,"Por favor use:\n",strlen("Por favor use:\n"));
		write(1,"aurras status -> Receber estado do processamento do servidor\n",strlen("aurras status -> Receber estado do processamento do servidor\n"));
		write(1,"aurras transform <inputFile> <outputFile> Filter1 Filter2 ... -> aplicação de filtros no inputFile\n",strlen("aurras transform <inputFile> <outputFile> Filter1 Filter2 ... -> aplicação de filtros no inputFile\n"));
	}
}
