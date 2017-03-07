#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> 
#include<string.h>
#include "functions.h"
#include<signal.h>
int main() 
{
	getcwd(home,sizeof(home));
	//signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
	//signal(EOF, sighup_handler);  /* ctrl-D */
	prompt();
	return 0;
}
