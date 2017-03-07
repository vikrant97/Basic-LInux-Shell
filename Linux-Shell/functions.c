#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<unistd.h> 
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include "functions.h"
#include<signal.h>
#include <fcntl.h>
#include <termios.h>
char *argsbg;
char *builtin_str[] = {
	"cd",
	"quit",
	"pwd",
	"echo",
	"pinfo",
	"jobs",
	"killallbg",
	"kjob",
	"fg",
};

typedef struct job_t {              
	int pid;              
	int jid;                
	int state;              
	char *cmdline; 
}job_t;
job_t job_list[100];
int max=0;
pid_t sigpid=0;
int (*builtin_func[]) (char **) = {
	&cd,
	&shellexit,
	&pwd,
	&f_echo,
	&pinfo,
	&list_jobs,
	&killall_bg,
	&kjob,
	&fg,
};
int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
int addjob(pid_t pid)
{

	job_list[++max].pid=pid;
	job_list[max].jid=max;
	job_list[max].state=0;
	job_list[max].cmdline=argsbg;
}
int killall_bg(char **args)
{
	int i=0;
	for(i=1;i<=max;i++)
	{
		if(job_list[i].state==0)
		{
			kill(job_list[i].pid,15);
			job_list[i].state=-1;
		}


	}
	max=0;
	return 1;

}
int kjob(char **args)
{
	if(args[1]==NULL || args[2]==NULL)
	{
		fprintf(stderr, "Error: Input format-> kjob <jobid> <signal>\n");
		return 0;
	}
	int jid=atoi(args[1]);
	if(jid>max)
	{
		fprintf(stderr, "Error job number does not exist\n");
		return 0;
	}

	int sig=atoi(args[2]);
	kill(job_list[jid].pid,sig);
	job_list[jid].state=-1;
	return 1;
}
int fg(char **args)
{
	if(args[1]==NULL)
	{
		fprintf(stderr, "Error: input format-> fg 'jobid'\n");
	}
	int jid=atoi(args[1]);
	printf("%s\n",job_list[jid].cmdline);
	sigpid=job_list[jid].pid;
	int status;
	pid_t wpid;
	job_list[jid].state=1;
	kill(job_list[jid].pid,SIGCONT);
	//printf("%s\n",job_list[jid].cmdline);
	do {
		wpid = waitpid(job_list[jid].pid, &status, WUNTRACED);
	} while(!WIFEXITED(status) && !WIFSIGNALED(status));


	return 1;
}
void sigtstp_handler(int sig) 
{
	int i;
	pid_t pid=0;
	if(sigpid)  // if pid ==0, i.e. no fg job, then return directly
	{
		if(kill(sigpid,SIGTSTP)) 
		{
			fprintf(stderr,"Error:Can't stop the process");
			return ;
		}
		else
		{
			addjob(sigpid);
			fprintf(stdout,"stopped\n");
			prompt();

		}
	}
	//printf("hi\n");
	return;
}
//void pipeHandler(char ** args);
void sigint_handler(int sig)
{

	int i;
	if(sigpid)  // if pid ==0, i.e. no fg job, then return directly
	{
		if(kill(sigpid,SIGINT))
			fprintf(stderr,"Error:Can't kill the process");
		else
		{

			printf("\n");

		}
	}
	prompt();
	return;
}
int cd(char **args)
{
	if(args[1]==NULL)
	{
		prompt();				
	}
	else if (chdir(args[1]) != 0) 
	{
		if(args[1]!=NULL)
			perror("lsh");
	}
	return 1;
}

void fileIO(char * args[], char* inputFile, char* outputFile, int option){

	int err = -1;
	pid_t pid;
	int fileDescriptor; // between 0 and 19, describing the output or input file

	if((pid=fork())==-1){
		printf("Child process could not be created\n");
		return;
	}
	if(pid==0){
		// Option 0: output redirection
		if (option == 0){
			// We open (create) the file truncating it at 0, for write only
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
			// We replace de standard output with the appropriate file
			dup2(fileDescriptor, STDOUT_FILENO); 
			close(fileDescriptor);
			// Option 1: input and output redirection
		}else if (option == 1){
			// We open file for read only (it's STDIN)
			fileDescriptor = open(inputFile, O_RDONLY, 0600);  
			// We replace de standard input with the appropriate file
			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor);
			// Same as before for the output file
			fileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(fileDescriptor, STDOUT_FILENO);
			close(fileDescriptor);		 
		}
		if (execvp(args[0],args)==err)
		{
			printf("err");
			kill(getpid(),SIGTERM);
		}		 
	}
	waitpid(pid,NULL,0);
}

void pipeHandler(char ** args){
	int filedes[2]; 
	int filedes2[2];

	int num_cmds = 0;

	char *command[256];
	char s[]="|";

	pid_t pid;

	int err = -1;
	int end = 0;
	int i=0;
	int j = 0;
	int k = 0;
	int l = 0;

	while (args[l] != NULL){
		if (strcmp(args[l],s) == 0){
			num_cmds++;
		}
		l++;
	}
	num_cmds++;

	while (args[j] != NULL && end != 1)
	{
		k = 0;
		while (strcmp(args[j],s) != 0)
		{
			command[k] = args[j];
			j++;	
			if (args[j] == NULL){
				end = 1;
				k++;
				break;
			}
			k++;
		}
		command[k] = NULL;
		j++;		
		if (i % 2 != 0)
		{
			pipe(filedes); 
		}
		else
		{
			pipe(filedes2); 
		}

		pid=fork();

		if(pid==-1)
		{			
			if (i != num_cmds - 1)
			{
				if (i % 2 != 0)
				{
					close(filedes[1]); // for odd i
				}
				else
				{
					close(filedes2[1]); // for even i
				} 
			}			
			printf("Child process could not be created\n");
			return;
		}
		if(pid==0)
		{
			if (i == 0){
				dup2(filedes2[1], STDOUT_FILENO);
			}
			else if (i == num_cmds - 1){
				if (num_cmds % 2 != 0){ // for odd number of commands
					dup2(filedes[0],STDIN_FILENO);
				}else{ // for even number of commands
					dup2(filedes2[0],STDIN_FILENO);
				}
			}else{ // for odd i
				if (i % 2 != 0){
					dup2(filedes2[0],STDIN_FILENO); 
					dup2(filedes[1],STDOUT_FILENO);
				}else{ // for even i
					dup2(filedes[0],STDIN_FILENO); 
					dup2(filedes2[1],STDOUT_FILENO);					
				} 
			}

			if (execvp(command[0],command)==err){
				kill(getpid(),SIGTERM);
			}		
		}

		if (i == 0)
		{
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1)
		{
			if (num_cmds % 2 != 0)
			{

				close(filedes[0]);
			}
			else
			{					
				close(filedes2[0]);
			}
		}
		else
		{
			if (i % 2 != 0)
			{					
				close(filedes2[0]);
				close(filedes[1]);
			}
			else
			{					
				close(filedes[0]);
				close(filedes2[1]);
			}
		}

		waitpid(pid,NULL,0);

		i++;	
	}
}


int pinfo(char ** args)
{
	char *x;
	int status;
	x=(args[1]);
	char line0[60],line6[60],line1[60],line2[60],line3[60],line4[60],line5[60];
	strcpy(line1,"cat /proc/");
	strcpy(line2,"/status | grep State");
	strcpy(line3,"/status | grep VmSize");
	strcat(line1,x);
	strcat(line1,line2);
	//printf("%s\n",line1);
	strcpy(line4,"/bin/bash");
	strcpy(line5,"-c");
	printf("PID-- %s\n",args[1]);
	char* args1[4];
	args1[0]=line4;
	args1[1]=line5;
	args1[2]=line1;
	args1[3]=NULL;
	char line7[60],line8[60];	
	pid_t pid,wpid; 
	pid= fork();
	if (pid == 0) {
		// Child process
		if (execvp(args1[0], args1) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} 
	else if (pid < 0) 
	{
		// Error forking
		perror("lsh");
	} 
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	//printf("hi\n");
	strcpy(line1,"cat /proc/");
	strcat(line1,x);
	strcat(line1,line3);
	args1[2]=line1;
	/*if (execvp(args1[0], args1) == -1) {
	  perror("lsh");
	  }*/
	pid=fork();
	if (pid == 0) {
		// Child process
		if (execvp(args1[0], args1) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} 
	else if (pid < 0) 
	{
		// Error forking
		perror("lsh");
	} 
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	strcpy(line0,"readlink");
	strcpy(line6,"-f");
	strcpy(line7,"/proc/");
	strcat(line7,x);
	strcpy(line8,"/exe");
	strcat(line7,line8);
	args1[0]=line0;
	args1[1]=line6;
	args1[2]=line7;
	pid=fork();
	if (pid == 0) {
		// Child process
		if (execvp(args1[0], args1) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} 
	else if (pid < 0) 
	{
		// Error forking
		perror("lsh");
	} 
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	prompt();
	return 1;
}
int shellexit(char **args)
{
	/*if (execvp(args[0], args) == -1) {
	  perror("lsh");
	  }*/
	exit(EXIT_FAILURE);
}
int pwd(char ** args)
{

	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	printf("Current Directory is: %s\n",cwd);
	//prompt();

}
int f_echo(char ** args)
{
	int i=1;
	while(args[i]!=NULL)
	{
		printf("%s ",args[i]);
		i++;
	}
	printf("\n");
	//prompt();
	return 1;
}
int list_jobs(char **args)
{
	int i=1;
	for(i=1;i<=max;i++)
	{
		if(job_list[i].state==0)
			printf("[%d] %s with PID [%d]\n",job_list[i].jid,job_list[i].cmdline,job_list[i].pid);

	}
	return 1;

}
char *read_line(void)
{
	char *line = NULL;
	ssize_t bufsize = 0; 
	getline(&line, &bufsize, stdin);
	return line;
}
char **split_line1(char *line)
{
	int bufsize = 10, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;
	token = strtok(line, ";\n");

	while (token != NULL) 
	{
		tokens[position] = token;
		position++;

		token = strtok(NULL, ";\n");
	}
	tokens[position] = NULL;
	return tokens;
}

char **split_line2(char *line)
{
	int bufsize = 10, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;
	token = strtok(line, " \t\r\n\a");

	while (token != NULL) 
	{
		tokens[position] = token;
		position++;

		token = strtok(NULL, " \t\r\n\a");
	}
	tokens[position] = NULL;
	return tokens;
}

int execute(char **args)
{
	int i=0;

	if (args[0] == NULL) {
		return 1;
	}
	char s1[]="|";
	while(args[i]!=NULL)
	{
		if(strcmp(args[i],s1)==0)
		{
			pipeHandler(args);
			return 1;
		}
		i++;
	}
	char s[]="<";
	char s2[]=">";
	char *args_aux[256];
	char *args_aux2[256];
	int j=0;
	while ( args[j] != NULL){
		if ( (strcmp(args[j],s) == 0) || (strcmp(args[j],s2) == 0) ){
			break;
		}
		args_aux[j] = args[j];
		j++;
	}
	i=0;
	while(args[i]!=NULL)
	{
		if (strcmp(args[i],s) == 0){
			int aux = i+1;
			if (args[aux] == NULL || args[aux+1]==NULL){
				printf("Not enough input arguments\n");
				return -1;
			}
			//else if(args[aux+1]==NULL)
			//	fileIO(args_aux,args[i+1],NULL,2);
			else if (strcmp(args[aux+1],s2) != 0 && strcmp(args[aux+1],s1)!=0){
					printf("Usage: Expected '>' and found %s\n",args[aux+1]);
					return -2;
				}
			//added
			/*else if(strcmp(args[i+2],s1)==0)
			{
				printf("hi\n");
				fileIO(args_aux,args,args[i+1],NULL,2);
				return 1;
			
			}*/
			
			fileIO(args_aux,args[i+1],args[i+3],1);
			return 1;
		}
		else if (strcmp(args[i],s2) == 0){
			if (args[i+1] == NULL){
				printf("Not enough input arguments\n");
				return -1;
			}
			fileIO(args_aux,NULL,args[i+1],0);
			return 1;
		}
		i++;
	}

	for (i = 0; i < num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}


	return launch(args);
}

int launch(char **args)
{
	pid_t pid, wpid;
	int status;
	int i=0,x=0;
	char s[]={"&"};

	while(args[i]!=NULL)
	{
		if(strcmp(args[i],&s[0])==0)
		{
			//	printf("hi\n");
			args[i]=NULL;
			x=1;
			break;
		}
		i++;
	}
	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(args[0], args) == -1) {
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} 
	else if (pid < 0) 
	{
		// Error forking
		perror("lsh");
	} 
	else {
		argsbg=args[0];
		if(x==1)
		{
			//addjob(pid);
			job_list[++max].pid=pid;
			job_list[max].jid=max;
			job_list[max].state=0;
			job_list[max].cmdline=args[0];

			printf("PID is %d\n",pid);
			//printf("%d\n",sigpid);
			prompt();
		}
		else
		{
			sigpid=pid;
			do {
				wpid = waitpid(pid, &status, WUNTRACED);
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
		}
		sigpid=0;
	}

	return 1;
}


void prompt()
{
	char *buf;
	signal(SIGINT,sigint_handler);
	signal(SIGTSTP,sigtstp_handler);
	buf=(char *)malloc(10*sizeof(char));
	buf=getlogin();
	printf("<%s@",buf);
	//username
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	printf("%s:", hostname);
	//username@hostname
	char cwd[1024];
	getcwd(cwd,sizeof(cwd));
	int flag=0,i=0;
	while(home[i]!='\0')
	{
		if(cwd[i]==home[i])
			i++;
		else
			flag=1;
	}
	if(flag==1)
		printf("%s>",cwd);
	else
	{
		printf("~");
		while(cwd[i]!='\0')
		{
			printf("%c",cwd[i]);
			i++;
		}
		printf(">");
	}
	int j,k=0;
	char * line=read_line();
	if (feof(stdin)) { 
	printf("\n");
			    fflush(stdout);
			   fflush(stderr);
			   exit(0);
	        }
	i=0;
	char **args=split_line1(line);
	while(args[i]!=NULL)
	{
		char **args2=split_line2(args[i]);
		i++;
		k=execute(args2);
	}
	prompt();
}
