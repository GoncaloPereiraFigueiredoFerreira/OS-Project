#include "aurrasd.h"

//Variaveis globais do servidor

int nproc = 0;
int flag = 1;
int fdIn; // ->fifo de entrada
int fdOut; // -->fifo de saida
unsigned char *proc[100][100] = {NULL}; //Contém: proc[index do processo na table][comandos do processo]
pid_t table[100] = {0};
int *semaphore;
FLTRS fltrs[64];
int nFilters=0;


//./aurrasd-filters/aurrasd-echo <../samples/sample-1-so.m4a> output.m4a

//./aurras transform <../samples/sample-1-so.m4a> output.m4a alto eco rapido

//codigo servidor



int initPipes (){

	mkfifo("pIn",0644); 				// pipe que recebe informaçao do cliente  //verifcar erros -1
	mkfifo("pOut",0644); 				// pipe que envia informaçao para o cliente
	fdIn = open("pIn",O_RDONLY);
	fdOut = open("pOut",O_WRONLY);

	semaphore = malloc(sizeof(int));
	*semaphore =0;
	return 0;
}


int endProgrm(){
	close(fdIn);
	close(fdOut);
	unlink("pIn");
	unlink("pOut");
}

int statusHandler(){
	//prints tasks on proc
	//prints filters
	//prints PID

	 write(fdOut,"Querias, queria batatas com enguias\0",37);
	 return 0;
}



int proc_args(unsigned char *l[]){
	//1º talvez contar o nº de argumentos
	int fdOut = open("pOut",O_WRONLY); // falta verificar erro -1
	int nargs=0;
	int j=0,i=0;
	while (l[nargs]!= NULL) nargs++;
	printf("Nº de args %d\n",nargs );

	if (nargs == 1 && strcmp("status",(char*) l[0]) == 0) statusHandler();
	
	else if (nargs >=3){
		if (strcmp("transform",(char*) l[0]) == 0){
			char *input = l[1]; 
			char *output = l[2];
			int found;
			for (i=0; i<nargs;i++)printf("%s\n",l[i]);
			/*for (i=3; i< nargs;i++){
				found=0;
				for ( j=0; j<nFilters && !found; j++){
					if (strcmp(fltrs[j]->name,(char*) l[i]) == 0){
						printf("Filtro: %s\n",fltrs[j]->name );
						printf("Filtro: %s\n",fltrs[j]->cmd );

						found=1;
					}
				}
				if (found != 0){
					while(fltrs[j]->max == fltrs[j]->curr);
					if (fork() == 0){
						if (i==3) { 
							printf("Filtro n %d\n",j);
							execlp(fltrs[j]->cmd,fltrs[j]->cmd,input,output,NULL);
						}
						else  {
							printf("Filtro n %d\n",j);
							execlp(fltrs[j]->cmd,fltrs[j]->cmd,output,output,NULL);
						}
					}
					else {
					// n sei se isto funciona sequer
						while ((*semaphore)!=0)printf("OLa\n");	//espera ativa
						*semaphore=1;
						fltrs[j]->curr++;
						printf("%d\n",fltrs[j]->curr);
						*semaphore =0;
						wait(NULL);
						while ((*semaphore)!=0) printf("OLa\n");
						*semaphore=1;
						fltrs[j]->curr--;	
						printf("%d\n",fltrs[j]->curr);
						
					}
				}

			}
		}
	}
	*/1
	sleep(2);
	return 0;
}

typedef void (*signalhandler_t) (int);

void handler_term(int n){
	int ret;
	flag = 0;
	for(int i = 0;i < nproc;i++) {wait(&ret);printf("%d\n",i);}

	//ach q depois disto deveria existir um _exit(), em vez da flag.
}


int readConfig(char * pathConfig,char * pathFilters){
	int fd = open(pathConfig,O_RDONLY);
	int bytes=0;
	char * buffer,*tmp;
	char *cmd;
	char * max;
	FLTRS new;

	if (fd == -1) perror("Erro no config file");

	do{
		buffer = read_line(fd,&bytes);
		if (bytes!= -1){
			tmp = buffer;
			new = malloc(sizeof(struct filters));
			new->name = strdup(strsep(&buffer," "));
			cmd = strsep(&buffer," ");
			new->cmd = malloc(sizeof(char) * (strlen(pathFilters)  + strlen(cmd)));
			new->cmd = strcat(new->cmd,pathFilters);
			new->cmd = strcat(new->cmd,cmd);
			max = strdup(strsep(&buffer,"\n\0"));
			sscanf(max,"%d",&new->max);
			new->curr=0;
			free(tmp);
			fltrs[nFilters] = new;
			nFilters++;
			printf("%s %s %d\n",new->name,new->cmd,new->max );
		}
	}while(bytes!=-1);

	return 0;
}





int main (int argc, char *argv[]){ // config-filename filters-folder
	//ler dos config files, e preencher "estruturas" para guardar os comandos
	if (argc == 3){

		initPipes(); //doesnt work
		
		char * pathConfig = argv[1];
		char * pathFilters = argv[2];
		readConfig(pathConfig,pathFilters);
		

		if (signal(SIGTERM,handler_term) == SIG_ERR) perror("SIGTERM error\n");
		pid_t pid;
		unsigned char buffer[PIPE_BUF];
		int nread,n = 0,ind,i,j;

		//initial read from pipe
		nread = read(fdIn,buffer,PIPE_BUF); //verificar erros -1

		while(flag){
			if(n == nread) {nread = read(fdIn,buffer,PIPE_BUF);n = 0;} //verificar erros -1


			if(nread > 0){
				pid = 0;
				for(i = 0;i < sizeof(pid_t);i++) 
					pid += buffer[i+n] << (8*i);

				n = n + sizeof(pid_t);

				ind = pid % 100;
				while(table[ind] && table[ind] != pid) ind = (ind + 1) % 100; //hashtable de processos

				for(i = 0; proc[ind][i];i++); 

				j = 0;

				fflush(stdout); //n percebi
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
					kill(pid,SIGTERM); //este term so devia ser mandado no proc args // verficar erro -1
					table[ind] = 0;
					_exit(1);
				}
				n++;
			}
		}
		close(fdOut); // verficar erro -1
		close(fdIn);
		unlink("pIn"); // verficar erro -1
	}
	return 0;
}