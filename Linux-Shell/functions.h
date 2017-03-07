int shellexit(char ** args);
int cd(char **args);
int pwd(char ** args);
int f_echo(char ** args);
int num_builtins();
char * read_line();
char **split_line1(char * line);
char **split_line2(char * line);
int execute(char **args);
int is_background(char **args);
int launch(char **args);
int pinfo(char **pid);
void prompt();
void pipHandler(char **args);
int list_jobs(char **args);
int killall_bg(char **args);
int kjob(char **args);
int fg(char **args);
void sighup_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
char home[1024];

//fileIO(char * args[], char* inputFile, char* outputFile, int option)
