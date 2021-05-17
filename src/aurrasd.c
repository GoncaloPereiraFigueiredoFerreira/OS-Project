#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

//codigo servidor

//quando for iniciado, lê o ficheiro de configs

int initPipes (){
	mkfifo("pIn",0777); // pipe que recebe informaçao do cliente  //verifcar erros
	mkfifo("pOut",0777); // pipe que envia informaçao para o cliente

}

//para o status precisamos de :
// um array de tasks a serem realizadas, ou seja guardar os argumentos recebidos
// retornar pid do servidor
// estado da utilizaçao maxima de cada filtro





int main (char *argv[], int argc){ // config-filename filters-folder
	initPipes();
	int fdIn = open("pIn",O_RDONLY);
	int fdOut = open("pOut",O_WRONLY);
	char bufferIN[PIPE_BUF];
	char bufferOUT[PIPE_BUF];
	char *bufferPROCESS[1024] = malloc(sizeof (char**));
	unsigned int i=0, counterP=0;



	if (signal(SIGTERM, handler) == SIG_ERR) perror("Failed"); //lidar com Sigterm
	

	while (1){


		read(fdIn,bufferIN,PIPE_BUF); // retorna erros
		for (; i < PIPE_BUF && bufferIN[i]!='\0'; i++){
			bufferPROCESS[counterP][i] = bufferIN[i];
		}
		if (bufferIN[i]=='\0'){

			i=0; counterP++;
		}
		else {

		}

		34(\n)ola2(\n)  34(\n)fdauif2njron(\n) 34(\t)fu2vb23tr(\n)   34(\n)ola43(\0) 




		// fazer um ciclo de intrepertaçao ate encontrar \0 no buffer
		// dividir argumentos, pid do cliente, filtros a aplicar, filepaths etc
		// criar um processo para aplicar estes filtros





	}
	close(fdIn);
	close(fdOut);





}