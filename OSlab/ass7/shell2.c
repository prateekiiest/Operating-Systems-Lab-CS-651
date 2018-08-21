#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define RESET "\e[0m"
#define BOLD "\e[1m"

// terminal colors 
#define YELLOW "\e[33m"
#define RED "\e[31m"

#define MAX_COMMAND_SIZE 1024 // to be updated this
#define CONTINUE_READING 1
#define MAX_SUBCOMMAND_SIZE 64 

#define FG 0x01 // code for foreground running process
#define BG 0x02 // code for background running process 
#define _STDOUT 0x04 // code for stdout
#define _STDERR 0x08 // code for stderr
#define _STDIN 0x10 // code for stdin

// Defining the commands  
int cd(char **args); //  command for executing the 'cd' command
int help(char **args); // command for executing the 'help' command
int exit_shell(char **args); // command for exitting the shell prompt
int pipe_execute(char** arg1, char** arg2); // command for executing the pipe functon between two arguments
int home(char **args);

// COMMANDS IMPLEMENTED TILL NOW
// External commands
char *command_str[] = {
  "cd",
  "help",
  "exit_shell",
  "home"  
};

void append(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

// IMPLEMENTED COMMAND FUNCTIONS
int (*command_func[]) (char **) = {
  &cd,
  &help,
  &exit_shell,
  &home
};

// FUNCTION FOR READING COMMANDS FROM COMMAND LINE
char *read_line(void){

	int command_size =  MAX_COMMAND_SIZE; // allocating the command size to maximum permissible command size
	int index = 0; // start position for the first command
    int ind2 = 0;
	int cmd;
	// ALLOCATION OF COMMANDS
	char *command = malloc(sizeof(char) * command_size);  // allocating space for the whole command
	if(!command){ //  if allocation not possible
		printf(RED"Could not allocate space for commands\n"RESET);
		exit(0); // terminate abnormally
}
 // while I continue reading from the command line
	while(CONTINUE_READING){
		cmd = getchar();  // Reading character by character from the command line
        
		if(cmd == ';'){ //  if it encounters a ';' split it into 2 halves
			command[index] = '\0';  
			return command;}
		
		if(cmd == '\n'){  // if end of line  reached, split the command
			command[index] = '\0';  
			return command;}    
		else{
			command[index] = cmd;	}
		index += 1; // iterate over while I continue reading
		}
  
	if(index>= command_size){
		printf(RED"Size of Command exceeded !!!! WARNING\n"RESET); 
	}  //  if the total command_size increases above the permissible command size
    return command;	
}

// Function for parsing the commands
char **parse_commands(char *commands){

	int sub_command_size = MAX_SUBCOMMAND_SIZE; // allocating maximum permissible subcommand size
	int index = 0;
	char **sub_cmds = malloc(sub_command_size * sizeof(char *));
	char *sub_command;
	sub_command = strtok(commands,"  \t\n");  // delimiter as space/ new tab/ new line
	while(sub_command!=NULL){
		sub_cmds[index] = sub_command;
		index += 1;
		sub_command = strtok(NULL,"  \t\n");
	}	
	sub_cmds[index]= NULL;	
	return sub_cmds;
}

int execute(char **args, int fd, int options){

    int bg = (options & BG) ? 1 : 0;
    int _stdout = (options & _STDOUT) ? 1 : 0;
    int _stderr = (options & _STDERR) ? 1 : 0;
    int _stdin = (options & _STDIN) ? 1 : 0;
    pid_t pid, wpid;
    int status;
    pid = fork();
    if( (pid) == 0 ) {
        // child process
        if(fd > 2) {
            if(_stdout && dup2(fd, STDOUT_FILENO) == -1 ) {
                fprintf(stderr, "Error duplicating stream: %s\n", strerror(errno));
                return 1; }
            if(_stderr && dup2(fd, STDERR_FILENO) == -1 ) {
                fprintf(stderr, "Error duplicating: %s\n" , strerror(errno));
                return 1;   }
            if(_stdin && dup2(fd, STDIN_FILENO) == -1 ) {
                fprintf(stderr, "Error duplicating: %s\n" , strerror(errno));
                return 1;  }
            close(fd); }
        if( execvp(args[0], args) == -1 ) {
            fprintf(stderr, RED"Bad command provided\n"RESET); }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        fprintf(stderr, " %s\n", strerror(errno));
    } else {
        do {
            if( !bg ) {
                wpid = waitpid(pid, &status, WUNTRACED);
            }
         } while ( !WIFEXITED(status) && !WIFSIGNALED(status) );
    }

return 1;
}


// Function for executing the commands
int commands_execute(char **args){
	int i;
    if(args[0] == NULL) { // if its a null or empty argument 
        return 1; }
    i = 0;
    while(args[i]) {  // else  go to the end of the command error
        i++; }
    
    // if command is already present in the character array, execute that
	int size = (sizeof(command_str)/sizeof(char *));
    for (i = 0; i < size; i++) { 
    if (strcmp(args[0], command_str[i]) == 0) { // if arg matches with the commands present under the character array
      return (*command_func[i])(args); }
  	}

	int j = 0;
    // while the args list is not empty or end of line    
    while(args[j] != NULL) { // check for the following cases
        // `>` operator  = open the file and write to it
        if( !strcmp(">", args[j]) ) {
            // the next command will indicate the file to be opened in write mode
            int fd = fileno(fopen(args[j+1], "w+")); // take the file descriptor of the file
            args[j] = NULL;
            return execute(args, fd, FG | _STDOUT); // execute the corresponding program
         }

        else if(!strcmp("||", args[j])){
            int fd = fileno(fopen(args[j+1], "r"));
            if(fd != -1){
                // if the command is not true (bad command)
                return execute(args,fd,FG);
                }
            else{
                printf(RED"Bad command provided !\n"RESET);
                }
            }
                  
        // `>>` operator = open the corresponding file and append to it 
        else if( !strcmp(">>", args[j]) ) {
            // the next sub-command - corresponding file gets opened and needs to be appended to
            int fd = fileno(fopen(args[j+1], "a+"));
            args[j] = NULL;
            return execute(args, fd, FG | _STDOUT);
        }
         // for `<` operator 
        else if( !strcmp("<", args[j]) ) {
            int fd = fileno(fopen(args[j+1], "r"));
            args[j] = NULL;
            return execute(args, fd, FG | _STDIN);
        }
        // for piping
        else if( !strcmp("|", args[j]) ) {
            char** arg2;
            int i = 0;
            args[j] = NULL;
            arg2 = &args[j+1];
            return pipe_execute(args, arg2);
        }
        j++;
    }
    return execute(args, STDOUT_FILENO, FG);
}
	
// Function for performing cd operation
int cd(char **args){
  if (args[1] == NULL){
    fprintf(stderr, "Enter Folder Name\n");
  } 
  else{
    if (chdir(args[1]) != 0){
      perror(RED"chdir"RESET);
    }
  }
  return 1;
}


int help(char **args){
  int i;
  int size = (sizeof(command_str)/sizeof(char *));
  printf("Type program names and arguments, and hit enter.\n");
  printf("Commands Implemented:\n");
  for (i = 0; i < size; i++) {
    printf("  %s\n", command_str[i]);  }
  printf(YELLOW"Use the man command for more information.\n"RESET);
  return 1;
  }

int exit_shell(char **args){
	return 0;}

// Function for performing pipe function with two arguments
int pipe_execute(char **arg1, char **arg2){
    int fd[2], pid;
    pipe(fd);
    int stdin_copy = dup(STDIN_FILENO);
    if( (pid = fork()) == 0 ) {
        close(STDOUT_FILENO);
        dup(fd[1]);
        close(fd[0]);
        execute(arg1, STDOUT_FILENO, FG);
        exit(EXIT_FAILURE);
    }
    else if (pid > 0){
        close(STDIN_FILENO);
        dup(fd[0]);
        close(fd[1]);
        execute(arg2, STDOUT_FILENO, FG);
        dup2(stdin_copy, STDIN_FILENO);
        return 1;
    }
}


char *getcwd(char *buf, size_t size);

void screen_design(void){

	system("clear");
	int i;
	for(i=0;i<8;i++){
		printf("\n");}
	printf(YELLOW"                               ==================================================\n"RESET);
	printf("\n");	
	printf("\033[22;34m                                             Shell    Implementation\033[0m\n");
	printf(BOLD"                                           Maintainer : Prateek Chanda\n"RESET);
	printf("\n");
	
    printf("                                             Available Functionalities\n");
    printf("                                           -----------------------------\n");
    printf("                                           *  Bash File Execution\n");
	printf("                                           * (External) Commands whose executable files are present \n");
    printf("                                           *  Internal Commands provided\n ");
    printf("                                          *  ;  ||  & && and combinations\n");
    printf("                                           * I/O Redirection : >> , > , < \n");
    printf("                                           *    Pipe Implementation \n");

    printf("\n");
 printf("                                           -----------------------------\n");

	printf("\033[22;34m                                  To See a list of available commands, type :help:\033[0m\n");
		printf("\n");
	printf(YELLOW"                                ===================================================\n"RESET);
	for(i=0;i<6;i++){
		printf("\n");}
	}


int home(char **args){ 
        screen_design();
}

int main(){
	char *commands; // commands read from the command line
	char **args;  // arguments (commands) taken from parsinng the commands
	int exec; // exec status = 1 or 0
	char cwd[1024]; // store the current directory
	// Call this function to design the screen
	screen_design();   
  
	do{
		printf(YELLOW"MyShell"RESET);	
		if (getcwd(cwd, sizeof(cwd)) != NULL)
			fprintf(stdout, BOLD":%s \033[22;34m$\033[0m"RESET, cwd);

        // Read the commands		
		commands = read_line();
       
        // parsing the commands from the command line
		args = parse_commands(commands);
       
         if(strcmp(*args,"mybash") == 0)
           { char *file_name = *args + strlen("mybash ");
            if(*file_name == NULL){
                printf(RED"No file provided:\n"RESET);}
            else{
                FILE *fptr;
                char ch;
                fptr = fopen(file_name, "r");
                if (fptr == NULL){
                printf(RED"Cannot open file \n"RESET);  
            }
            char cmd[25] = "";
            ch = fgetc(fptr);
            int i = 0;
            while (ch != EOF)
            {
                append(cmd, ch);
                ch = fgetc(fptr);
                
            }
            fclose(fptr);
            //printf("Commands are %s\n",cmd);
            args = parse_commands(cmd);      
            exec = commands_execute(args);
		    free(commands);
	        free(args);
                    }
           }
        // exec the commands and return the status
		else{            
            exec = commands_execute(args);
		    free(commands);
	        free(args);
            }


           
            
	}while(exec);
	

	return 0;
}