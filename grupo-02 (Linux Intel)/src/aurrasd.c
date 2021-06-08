#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>


int nproc = 0,flag = 1;
//codigo servidor

//quando for iniciado, lê o ficheiro de configs

int initPipes (){
	mkfifo("pIn",0777); // pipe que recebe informaçao do cliente  //verifcar erros
	mkfifo("pOut",0777); // pipe que envia informaçao para o cliente
	return 0;
}


int proc_args(unsigned char *l[]){
	//1º talvez contar o nº de argumentos
	int fdOut = open("pOut",O_WRONLY);
	int nargs=0;
	while (l[nargs]!= NULL) nargs++;
	printf("Nº de args %d\n",nargs );

	if (nargs == 1 && strcmp("status",(char*) l[0]) == 0) write(fdOut,"Querias, queria batatas com enguias\0",37);
	else if (nargs >=3){
		if (strcmp("transform",(char*) l[0]) == 0){
			write(fdOut,"és crente filho\0",17);
		}
	}


	//unlink("pOut");
	sleep(2);
	return 0;
}

typedef void (*signalhandler_t) (int);

void handler_term(int n){
	int ret;
	flag = 0;
	for(int i = 0;i < nproc;i++) {wait(&ret);printf("%d\n",i);}
}
//para o status precisamos de :
// um array de tasks a serem realizadas, ou seja guardar os argumentos recebidos
// retornar pid do servidor
// estado da utilizaçao maxima de cada filtro




int main (int argc, char *argv[]){ // config-filename filters-folder
	initPipes();

	signal(SIGTERM,handler_term);

	int fdIn = open("pIn",O_RDONLY);
	int fdOut = open("pOut",O_WRONLY);
	pid_t pid;
	unsigned char buffer[PIPE_BUF],*proc[100][100] = {NULL};
	pid_t table[100] = {0};
	int nread,n = 0,ind,i,j;

	//initial read from pipe
	nread = read(fdIn,buffer,PIPE_BUF);

	while(flag){
		if(n == nread) {nread = read(fdIn,buffer,PIPE_BUF);n = 0;}

		//pid = ((void*)buffer)+n;
		if(nread > 0){
			pid = 0;
			for(i = 0;i < sizeof(pid_t);i++) 
				pid += buffer[i+n] << (8*i);

			n = n + sizeof(pid_t);

			ind = pid % 100;
			while(table[ind] && table[ind] != pid) ind = (ind + 1) % 100; //hashtable de processos

			for(i = 0; proc[ind][i];i++); //n percebi 

			j = 0;

			fflush(stdout);
			if (buffer[n] == '\n'){
				proc[ind][i] = malloc(sizeof(char) * 512);
			}
			else for(;proc[ind][i][j] != '\0';j++);
			n++;

			for(;buffer[n] != '\0' && buffer[n] != '\n';j++,n++) 
				proc[ind][i][j] = buffer[n];
			proc[ind][i][j] = '\0';

			if(buffer[n] == '\0' && !fork()){
				proc_args(proc[ind]);
				for(i = 0;proc[ind][i];i++){
					free(proc[ind][i]);
					proc[ind][i] = NULL;
				}
				kill(pid,SIGTERM); //este term so devia ser mandado no proc args
				table[ind] = 0;
				_exit(1);
			}
			n++;
		}
	}
	close(fdOut);
	close(fdIn);
	unlink("pIn");

}