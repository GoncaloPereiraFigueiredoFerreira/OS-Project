#include "aurrasd.h"

//Variaveis globais do servidor

int nproc = 0;
int flag = 1;
int fdIn; // ->fifo de entrada
int fdOut; // -->fifo de saida
unsigned char *proc[100][100] = {NULL}; //Contém: proc[index do processo na table][comandos do processo]
pid_t table[100] = {-1};
int waitQueue[100] = {-1};
int nWait = 0;
FLTRS fltrs[64];
int nFilters=0;


//./aurrasd-filters/aurrasd-echo <../samples/sample-1-so.m4a> output.m4a

//./aurras transform <../samples/sample-1-so.m4a> output.m4a alto eco rapido

//./aurrasd ../etc/aurrasd.conf aurrasd-filters/


//codigo servidor



int initPipes (){

	mkfifo("pIn",0644); 				// pipe que recebe informaçao do cliente  //verifcar erros -1
	mkfifo("pOut",0644); 				// pipe que envia informaçao para o cliente
	fdIn = open("pIn",O_RDONLY);
	fdOut = open("pOut",O_WRONLY);

	return 0;
}


int endProgrm(){
	close(fdIn);
	close(fdOut);
	unlink("pIn");
	unlink("pOut");
	return 0;
}

int statusHandler(){
	//prints tasks on proc
	char tmp[1024];
	int i=0,j=0,count=0;
	for (i=0; i< 100; i++){
		count=0;
		while (proc[i][count]!= NULL) count++;
		if (count>=1){
			sprintf(tmp,"task #%d:",i);
			j=0;
			while(j<count) {strcat(tmp," ");strcat(tmp, (char *) proc[i][j]); j++;}
			strcat(tmp,"\n");
			write(fdOut,tmp,strlen(tmp)*sizeof(char));
		}

	}
	//prints filters
	
	for (i=0, j=0; i<nFilters; j++){
		if (fltrs[j]!= NULL){
			write(fdOut,"filter ",strlen("filter ") * sizeof(char));
			write(fdOut,fltrs[j]->name,strlen(fltrs[j]->name) * sizeof(char));
			sprintf(tmp,": %d/%d (running/max)\n",fltrs[j]->curr,fltrs[j]->max);
			write(fdOut,tmp,strlen(tmp) * sizeof(char));
			i++;
		}
	}
	//prints PID
	sprintf(tmp, "pid: %d", (int) getpid());
	write(fdOut,tmp,strlen(tmp)*sizeof(char));
	//write(fdOut,"Querias, queria batatas com enguias\0",37);
	return 0;
}



int proc_args(int ind,pid_t pid){
	//1º talvez contar o nº de argumentos
	int nargs=0;
	int j=0,i=0,n = 0,nfil[nFilters];
	char **l = (char**) proc[ind];
	while (l[nargs]!= NULL) {printf("%s\n",l[nargs]);nargs++;}
	printf("Nº de args %d\n",nargs);
	int pipes[2];
	int beforepipe = 0;

	for(i = 0;i < nFilters;i++)nfil[i] = 0;

	if (nargs == 1 && strcmp("status",(char*) l[0]) == 0) {
		statusHandler();
		for(i = 0;proc[ind][i];i++){
			free(proc[ind][i]);
			proc[ind][i] = NULL;
		}
		kill(pid,SIGTERM); //este term so devia ser mandado no proc args // verficar erro -1
		table[ind] = 0;
		return 0;
	}	

	else if (nargs >=3){

		for (i = 0; i < nFilters;i++){
			for(j = 0; j < nargs;j++){
				if(strcmp(fltrs[i]->name,l[j])){
					if(fltrs[i]->curr < fltrs[i]->max){
						nfil[i]++;
					}
					else n = 1; //falg para saber se o max de algum filtro e passado 
				}
			}
		}

		for (i = 0; i < nFilters;i++){
			if(n && nfil[i] > fltrs[i]->max){
				//tratar de mandar para a merda
			}
			else fltrs[i]->curr += nfil[i];
		}
		if(n){
			for(i = nWait;waitQueue[i] != -1;nWait = (nWait + 1)%100);
			waitQueue[i] = ind;
		}
		nproc++;

		if (strcmp("transform",(char*) l[0]) == 0 && !fork()){
			int input,output;
			if((input = open(l[1],O_RDONLY)) == -1) perror("erroin\n"); 
			if((output = open(l[2],O_CREAT|O_WRONLY|O_TRUNC,0640)) == -1) perror("errooff\n");
			int found;
			for (i=0; i<nargs;i++)printf("%s\n",l[i]);
			for (i=3; i< nargs;i++){
				found=0;
				for ( j=0; j<nFilters && !found; j++){
					if (strcmp(fltrs[j]->name,(char*) l[i]) == 0){
						printf("%s  |   %s\n",fltrs[j]->name,fltrs[j]->cmd);
						found=1;
					}
				}
				if (found != 0){
					pipe(pipes);
					j--;
					if (fork() == 0){
						close(pipes[0]);
						if(i == 3 && i == nargs-1){
							close(pipes[1]);
							dup2(input,0);
							dup2(output,1);
						}
						else if (i==3) {
							dup2(input,0);
							dup2(pipes[1],1);
							close(pipes[1]);
						}
						else if (i == nargs-1){
							dup2(beforepipe,0);
							dup2(output,1);
							close(pipes[1]);
							close(beforepipe);
						}
						else {
							dup2(beforepipe,0);
							dup2(pipes[1],1);
							close(pipes[1]);
							close(beforepipe);
						}
						if(execlp(fltrs[j]->cmd,fltrs[j]->cmd,NULL) == -1)
							{perror(fltrs[j]->cmd);_exit(0);}
					}
					if (beforepipe)close(beforepipe);
					beforepipe = pipes[0];
					close(pipes[1]);
				}
			}
			close(pipes[0]);
			for(i = 3; i < nargs; i++) {wait(NULL);printf("Acabou i->%d\n",i);}
			close(input);
			close(output);
		}
		else return 0;
	}

	free(proc[ind][0]);
	proc[ind][0] = NULL;
	kill(pid,SIGTERM); //este term so devia ser mandado no proc args // verficar erro -1
	kill(getppid(),SIGINT);
	_exit(1);
}

typedef void (*signalhandler_t) (int);

void handler_term(int n){
	endProgrm();
	int ret;
	flag = 0;
	for(int i = 0;i < nproc;i++) {wait(&ret);printf("%d\n",i);}

	//ach q depois disto deveria existir um _exit(), em vez da flag.
}

void int_handler(int n){
	printf("ola\n");
	for(int i = 0; i < 100;i++){
		if(table[i] != 0 && proc[i][0] == NULL){
			for(int j = 3; proc[i][j] != NULL;j++){
				for(int k = 0; k < nFilters;k++){
					if(!(strcmp(fltrs[k]->name,(char *)proc[i][j]))) {
						fltrs[k]->curr--;
					}
				}
				if(proc[i][j])
					free(proc[i][j]);
				proc[i][j] = NULL;
			}
			free(proc[i][1]);
			free(proc[i][2]);
			proc[i][1] = NULL;
			proc[i][2] = NULL;
			table[i] = -1;
			nproc--;
		}
	}
	if(waitQueue[nWait] != -1){proc_args(waitQueue[nWait],table[waitQueue[nWait]]);}
	waitQueue[nWait] = -1;
	nWait = (nWait + 1) % 100;
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
			cmd = strdup(strsep(&buffer," "));
			new->cmd = malloc(sizeof(char) * (strlen(pathFilters)  + strlen(cmd) + 20));
			new->cmd = strcat(new->cmd,pathFilters);
			new->cmd = strcat(new->cmd,cmd);
			printf("cmd -> %s  ||  NewCmd-> %s\n",cmd,new->cmd);
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

		initPipes();
		
		char * pathConfig = argv[1];
		char * pathFilters = argv[2];
		readConfig(pathConfig,pathFilters);
		

		if (signal(SIGTERM,handler_term) == SIG_ERR) perror("SIGTERM error\n");
		if (signal(SIGINT,int_handler) == SIG_ERR) perror("SIGINT error\n");
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

				if (buffer[n] == '\n'){
					proc[ind][i] = malloc(sizeof(char) * 512);
				}
				else for(;proc[ind][i][j] != '\0';j++);
				n++;

				for(;buffer[n] != '\0' && buffer[n] != '\n';j++,n++) 
					proc[ind][i][j] = buffer[n];
				proc[ind][i][j] = '\0';

				if(buffer[n] == '\0'){
					proc_args(ind,pid);
				}
				n++;
			}
		}
		endProgrm();
	}
	return 0;
}
