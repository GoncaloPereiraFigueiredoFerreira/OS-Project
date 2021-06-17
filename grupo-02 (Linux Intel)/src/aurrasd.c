#include "aurrasd.h"

//Variaveis globais do servidor

int nproc = 0;
int flag = 1;
int fdIn; // ->fifo de entrada
int fdOut; // -->fifo de saida
int fdAux; // --->abro fifo de leitura para bloquear
unsigned char *proc[100][100] = {NULL}; //Contém: proc[index do processo na table][comandos do processo]
pid_t table[100] = {0};
int waitQueue[100];
int nWait = 0;
FLTRS fltrs[64];
int nFilters=0;


//./aurrasd-filters/aurrasd-echo <../samples/sample-1-so.m4a> output.m4a

//./aurras transform <../samples/sample-1-so.m4a> output.m4a alto eco rapido

//./aurrasd ../etc/aurrasd.conf aurrasd-filters/


//codigo servidor



int initPipes (){

	mkfifo("tmp/pIn",0644); 				// pipe que recebe informaçao do cliente  //verifcar erros -1
	mkfifo("tmp/pOut",0644); 				// pipe que envia informaçao para o cliente
	if((fdIn = open("tmp/pIn",O_RDONLY))==-1)perror("Open");
	if((fdOut = open("tmp/pOut",O_WRONLY))==-1)perror("Open");
	if((fdAux = open("tmp/pIn",O_WRONLY))==-1)perror("open");
	memset(waitQueue,-1,100 * sizeof(int));
	return 0;
}


int endProgrm(){
	if(close(fdIn)==-1)perror("Close");
	if(close(fdOut)==-1)perror("Close");
	if(close(fdAux)==-1)perror("Close");
	if(unlink("tmp/pIn")==-1)perror("Unlink");
	if(unlink("tmp/pOut")==-1)perror("Unlink");
	for(int i = 0;i < nFilters;i++){
		free(fltrs[i]->name);
		free(fltrs[i]->cmd);
		free(fltrs[i]);
	}
	return 0;
}

void statusHandler(){
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
			if(write(fdOut,tmp,strlen(tmp)*sizeof(char))==-1)perror("Write");
		}

	}
	//prints filters
	
	for (i=0, j=0; i<nFilters; j++){
		if (fltrs[j]!= NULL){
			if(write(fdOut,"filter ",strlen("filter ") * sizeof(char))==-1)perror("Write");
			if(write(fdOut,fltrs[j]->name,strlen(fltrs[j]->name) * sizeof(char))==-1)perror("Write");
			sprintf(tmp,": %d/%d (running/max)\n",fltrs[j]->curr,fltrs[j]->max);
			if(write(fdOut,tmp,strlen(tmp) * sizeof(char))==-1)perror("Write");
			i++;
		}
	}
	//prints PID
	sprintf(tmp, "pid: %d", (int) getpid());
	if(write(fdOut,tmp,strlen(tmp)*sizeof(char))==-1)perror("Write");
	//write(fdOut,"Querias, queria batatas com enguias\0",37);
}



int proc_args(int ind,pid_t pid){
	//1º talvez contar o nº de argumentos
	int nargs=0;
	int j=0,i=0,n = 0,nfil[nFilters];
	char **l = (char**) proc[ind];
	while (l[nargs]!= NULL) {
		//printf("%s\n",l[nargs]);
		nargs++;
	}
	//printf("Nº de args %d\n",nargs);
	int pipes[2];
	int beforepipe = 0;
	pid_t aux;

	for(i = 0;i < nFilters;i++)nfil[i] = 0;

	if (nargs == 1 && strcmp("status",(char*) l[0]) == 0) {
		statusHandler();
		for(i = 0;proc[ind][i];i++){
			free(proc[ind][i]);
			proc[ind][i] = NULL;
		}
		if(kill(pid,SIGTERM)==-1)perror("Kill"); //este term so devia ser mandado no proc args // verficar erro -1
		table[ind] = 0;
		return 0;
	}	

	else if (nargs >=3){
		for (i = 0; i < nFilters;i++){
			for(j = 3; j < nargs;j++){
				if(!strcmp(fltrs[i]->name,l[j])){
					nfil[i]++;
					//printf("nfil -> %d > %d\n",(fltrs[i]->curr)+nfil[i],(fltrs[i]->max));
					if(fltrs[i]->curr + nfil[i] > (fltrs[i]->max)) n = 1; //falg para saber se o max de algum filtro e passado 
				}
			}
		}
		for (i = 0; i < nFilters;i++){
			if(n && nfil[i] > fltrs[i]->max){
				if(kill(pid,SIGUSR1)==-1)perror("Kill");
				//printf("exeço\n");
				table[ind] = 0;
				for(j = 0;proc[ind][j] != NULL; j++){
					free(proc[ind][j]);
					proc[ind][j] = NULL;
				}
				return 0;
			}
			else if(!n){
				fltrs[i]->curr += nfil[i];
			//printf("nfil -> %s -> %d\n",fltrs[i]->name,nfil[i]);
			}
		}
		nproc++;
		if(n){
			//printf("Entrou na queue\n");
			for(i = nWait;waitQueue[i] != -1;i = (i + 1)%100);
			waitQueue[i] = ind;
			return 0;
		}

		if (strcmp("transform",(char*) l[0]) == 0){
			kill(pid,SIGUSR2);
			table[ind] = -1;
			//printf("free\n");
		}
		if((aux = fork())==-1)perror("Fork"); 
		if(!aux){
			int input,output;
			if((input = open(l[1],O_RDONLY)) == -1) perror("erroin\n"); 
			if((output = open(l[2],O_CREAT|O_WRONLY|O_TRUNC,0640)) == -1) perror("erroout\n");
			int found;
			//for (i=1; i<nargs;i++)//printf("%s\n",l[i]);
			for (i=3; i< nargs;i++){
				found=0;
				for ( j=0; j<nFilters && !found; j++){
					if (strcmp(fltrs[j]->name,(char*) l[i]) == 0){
						//printf("%s  |   %s\n",fltrs[j]->name,fltrs[j]->cmd);
						found=1;
					}
				}
				if (found != 0){
					if(pipe(pipes)==-1)perror("Pipe");
					j--;
					if((aux = fork()) == -1)perror("Fork");
					if (!aux){
						if(close(pipes[0])==-1)perror("Close");
						if(i == 3 && i == nargs-1){
							if(close(pipes[1])==-1)perror("Close");
							if(dup2(input,0)==-1)perror("Dup");
							if(dup2(output,1)==-1)perror("Dup");
						}
						else if (i==3) {
							if(dup2(input,0)==-1)perror("Dup");
							if(dup2(pipes[1],1)==-1)perror("Dup");
							if(close(pipes[1])==-1)perror("Close");
						}
						else if (i == nargs-1){
							if(dup2(beforepipe,0)==-1)perror("Dup");
							if(dup2(output,1)==-1)perror("Dup");
							if(close(pipes[1])==-1)perror("Close");
							if(close(beforepipe)==-1)perror("Close");
						}
						else {
							if(dup2(beforepipe,0)==-1)perror("Dup");
							if(dup2(pipes[1],1)==-1)perror("Dup");
							if(close(pipes[1])==-1)perror("Close");
							if(close(beforepipe)==-1)perror("Close");
						}
						if(execlp(fltrs[j]->cmd,fltrs[j]->cmd,NULL) == -1)
							{perror(fltrs[j]->cmd);_exit(0);}
					}
					if (beforepipe) if(close(beforepipe)==-1)perror("Close");
					beforepipe = pipes[0];
					if(close(pipes[1])==-1)perror("Close");
				}
			}
			if(close(pipes[0])==-1)perror("Close");
			for(i = 3; i < nargs; i++) {
				wait(NULL);
				//printf("Acabou i->%d\n",i);
			}
			if(close(input)==-1)perror("Close");
			if(close(output)==-1)perror("Close");
			if(kill(pid,SIGTERM)==-1)perror("Kill"); //este term so devia ser mandado no proc args // verficar erro -1
			if(kill(getppid(),SIGINT)==-1)perror("Kill");
			_exit(1);
		}
		else return 0;
	}
	return 0;
}

typedef void (*signalhandler_t) (int);

void handler_term(int n){
	endProgrm();
	int ret;
	flag = 0;
	for(int i = 0;i < nproc;i++) {wait(&ret);//printf("nproc -> %d\n",nproc);
	}
	//ach q depois disto deveria existir um _exit(), em vez da flag.
}

void int_handler(int n){
	for(int i = 0; i < 100;i++){
		if(table[i] == -1){
			for(int j = 3; proc[i][j] != NULL;j++){
				for(int k = 0; k < nFilters;k++){
					if(!(strcmp(fltrs[k]->name,(char *)proc[i][j]))) {
						fltrs[k]->curr--;
						//printf("%s->curr = %d\n",fltrs[k]->name,fltrs[k]->curr);
					}
				}
				if(proc[i][j])
					free(proc[i][j]);
				proc[i][j] = NULL;
			}
			free(proc[i][0]);
			free(proc[i][1]);
			free(proc[i][2]);
			proc[i][0] = NULL;
			proc[i][1] = NULL;
			proc[i][2] = NULL;
			table[i] = 0;
			nproc--;
		}
	}
	if(waitQueue[nWait] != -1){
		//printf("Saiu da queu!\n");
		proc_args(waitQueue[nWait],table[waitQueue[nWait]]);
		waitQueue[nWait] = -1;
		nWait = (nWait + 1) % 100;
	}
}


int readConfig(char * pathConfig,char * pathFilters){
	int fd;
	if((fd = open(pathConfig,O_RDONLY))==-1)perror("Open");
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
			if((new = malloc(sizeof(struct filters)))==(void *)-1)perror("Malloc");
			new->name = strdup(strsep(&buffer," "));
			cmd = strdup(strsep(&buffer," "));
			if((new->cmd = malloc(sizeof(char) * (strlen(pathFilters)  + strlen(cmd) + 1)))==(void *)-1)perror("Malloc");
			new->cmd = strcat(new->cmd,pathFilters);
			new->cmd = strcat(new->cmd,cmd);
			//printf("cmd -> %s  ||  NewCmd-> %s\n",cmd,new->cmd);
			max = strdup(strsep(&buffer,"\n\0"));
			sscanf(max,"%d",&new->max);
			new->curr=0;
			fltrs[nFilters] = new;
			nFilters++;
			//printf("%s %s %d\n",new->name,new->cmd,new->max );
			free(cmd);
			free(max);
			free(tmp);
		}
	}while(bytes!=-1);
	free(buffer);
	return 0;
}





int main (int argc, char *argv[]){ // config-filename filters-folder
	//ler dos config files, e preencher "estruturas" para guardar os comandos
	if (argc == 3){
		char * pathConfig = argv[1];
		char * pathFilters = argv[2];
		readConfig(pathConfig,pathFilters);
		initPipes();
		


		if (signal(SIGTERM,handler_term) == SIG_ERR) perror("SIGTERM error\n");
		if (signal(SIGINT,int_handler) == SIG_ERR) perror("SIGINT error\n");
		pid_t pid;
		unsigned char buffer[PIPE_BUF];
		int nread,n = 0,ind,i,j;

		//initial read from pipe
		if((nread = read(fdIn,buffer,PIPE_BUF))==-1)perror("Read"); //verificar erros -1

		while(flag){
			if(n == nread) {
				if((nread = read(fdIn,buffer,PIPE_BUF))==-1)perror("Read");
				n = 0;
			}

			if(nread > 0){
				pid = 0;
				for(i = 0;i < sizeof(pid_t);i++) 
					pid += buffer[i+n] << (8*i);

				n = n + sizeof(pid_t);

				ind = pid % 100;
				if(buffer[n] == '\r'){
					while(table[ind]) ind = (ind + 1) % 100;
					table[ind] = pid;
				}
				else while(table[ind] != pid) ind = (ind + 1) % 100; //hashtable de processos

				for(i = 0; proc[ind][i];i++); 

				j = 0;


				if (buffer[n] == '\n' || buffer[n] == '\r'){
					if((proc[ind][i] = malloc(sizeof(char) * 512))==(void *)-1)perror("Malloc");
				}
				else for(;proc[ind][i][j] != '\0';j++);
				n++;

				for(;buffer[n] != '\0' && buffer[n] != '\n';j++,n++) 
					proc[ind][i][j] = buffer[n];
				proc[ind][i][j] = '\0';

				if(buffer[n] == '\0'){
					//printf("pid -> %d\n",pid);
					proc_args(ind,pid);
				}
				n++;
			}
		}
	}
	return 0;
}
