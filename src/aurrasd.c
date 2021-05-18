#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

//codigo servidor

//quando for iniciado, lê o ficheiro de configs

int initPipes (){
	mkfifo("pIn",0777); // pipe que recebe informaçao do cliente  //verifcar erros
	mkfifo("pOut",0777); // pipe que envia informaçao para o cliente
	return 0;
}

//para o status precisamos de :
// um array de tasks a serem realizadas, ou seja guardar os argumentos recebidos
// retornar pid do servidor
// estado da utilizaçao maxima de cada filtro




int main (int argc, char *argv[]){ // config-filename filters-folder
	initPipes();

	int fdIn = open("pIn",O_RDONLY);
	//int fdOut = open("pOut",O_WRONLY);
	char bufferIN[PIPE_BUF];
	char bufferOUT[PIPE_BUF];
	pid_t pids[100] = {0};
	pid_t tmpid;
	char * args[100][100] = {NULL}; 
	int retRead;
	unsigned int i=0, counterP=0, j=0, k=0, l=0;
	unsigned int *pcounterP= &counterP;



	//if (signal(SIGTERM, handler) == SIG_ERR) perror("Failed"); //lidar com Sigterm
	
	retRead = read(fdIn,bufferIN,PIPE_BUF);
	
	while (1){

		if (i == retRead){ retRead = read(fdIn,bufferIN,PIPE_BUF); i=0;}

		tmpid = (pid_t) (((void*)bufferIN)+i);
		printf("pid = %d\n",tmpid);

		i+= sizeof(pid_t);

		for (j=0;  pids[j] != tmpid && j < counterP;) if ( pids[j] != 0) j++; 

		if (pids[j] != tmpid){
			pids[j]= tmpid;
			counterP++;}

		//adicionar restante comando
		for (k=0; args[j][k] != NULL; k++);
			
		if (bufferIN[i]== '\t'){
			k--;
			for (l=0; l<1024 && args[j][k][l] != '\0';l++);

		}
		else if (bufferIN[i]== '\n') {
			args[j][k] = malloc(sizeof(char)*1024);
			l=0;
		}
		
		i++;


		for (; i< PIPE_BUF && (bufferIN[i] != '\n' || bufferIN[i] != '\0'); i++, l++ )
			args[j][k][l] = bufferIN[i];

		args[j][k][l] = '\0';
		printf("l= %d\n", l);


		if (bufferIN[i] != '\0' && fork() == 0) {
				//process(args[j],tmpid);
				printf("%s %s %s",args[j][0],args[j][1],args[j][2]);
				//send signl q ja acabou
				//(*pcounterP)--;
				//limpa os args
			}


		// ler pedaço de comando
		// associa pedaço de comando a pid
		// quando completo começar o processo do mesmo


		//34(\n)ola2(\n)  34(\n)fdauif2njron(\n) 34(\t)fu2vb23tr(\n)   34(\n)ola43(\0) 

		// fazer um ciclo de intrepertaçao ate encontrar \0 no buffer
		// dividir argumentos, pid do cliente, filtros a aplicar, filepaths etc
		// criar um processo para aplicar estes filtros





	}
	close(fdIn);
	//close(fdOut);

	return 0;



}