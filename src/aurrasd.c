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
	pid_t pid;
	unsigned char buffer[PIPE_BUF],*proc[100][100] = {NULL};
	pid_t table[100] = {0};
	int nread,n = 0,ind,i,j;

	nread = read(fdIn,buffer,PIPE_BUF);

	while(1){
		if(n == nread) {nread = read(fdIn,buffer,PIPE_BUF);n = 0;}

		//pid = ((void*)buffer)+n;
		if(nread > 0){
			pid = 0;
			for(i = 0;i < sizeof(pid_t);i++){pid += buffer[i+n] << (8*i);printf("%d\n",buffer[n+i]);}
			printf("buffer -> %s |pid -> %d | n -> %d | nread -> %d\n",buffer,pid,n,nread);
			n += sizeof(pid_t);
			printf("%d\n",buffer[n]);

			ind = pid % 100;
			while(table[ind] && table[ind] != pid) ind = (ind + 1) % 100;

			for(i = 0; proc[ind][i];i++);

			j = 0;

			if (buffer[n] == '\n'){
				proc[ind][i] = malloc(sizeof(char) * 512);
			}
			else for(;proc[ind][i][j] != '\0';j++);
			n++;

			for(;buffer[n] != '\0' && buffer[n] != '\n';j++,n++) 
				proc[ind][i][j] = buffer[n];
			proc[ind][i][j] = '\0';
			printf("arg -> %s\n" ,proc[ind][i]);

			if(buffer[n] = '\0');
			n++;
		}
	}

}